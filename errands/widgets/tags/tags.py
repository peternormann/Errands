# Copyright 2024 Vlad Krupinskii <mrvladus@yandex.ru>
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os

from gi.repository import Adw, GObject, Gtk  # type:ignore

from errands.lib.data import UserData
from errands.lib.utils import get_children
from errands.state import State


@Gtk.Template(filename=os.path.abspath(__file__).replace(".py", ".ui"))
class Tags(Adw.Bin):
    __gtype_name__ = "Tags"

    tags_list: Gtk.ListBox = Gtk.Template.Child()

    def __init__(self):
        super().__init__()
        State.tags_page = self
        # Load tags
        for tag in UserData.tags:
            self.tags_list.append(Tag(tag.text, self))
        self.update_ui()

    @property
    def tags(self) -> list[Tag]:
        return get_children(self.tags_list)

    def update_ui(self, update_tag_rows: bool = True):
        tags: list[str] = [t.text for t in UserData.tags]

        # Remove tags
        for row in self.tags:
            if row.get_title() not in tags:
                self.tags_list.remove(row)

        # Add tags
        tags_rows_texts: list[str] = [t.get_title() for t in self.tags]
        for tag in tags:
            if tag not in tags_rows_texts:
                self.tags_list.append(Tag(tag, self))

        # Set activatable rows
        if update_tag_rows:
            tags_in_tasks = [t.tags for t in UserData.tasks if t.tags]
            for tag in self.tags:
                tag.update_ui(tags_in_tasks)

        self.tags_list.set_visible(len(self.tags) > 0)
        if State.main_window:
            State.tags_sidebar_row.update_ui()

    @Gtk.Template.Callback()
    def _on_tag_added(self, entry: Adw.EntryRow):
        text: str = entry.get_text().strip().strip(",")
        if text.strip(" \n\t") == "" or text in [t.text for t in UserData.tags]:
            return
        self.tags_list.append(Tag(text, self))
        UserData.add_tag(text)
        entry.set_text("")
        self.update_ui()


class Tag(Adw.ActionRow):
    def __init__(self, title: str, tags: Tags):
        super().__init__()
        self.tags = tags
        self.set_title(title)
        delete_btn = Gtk.Button(
            icon_name="errands-trash-symbolic",
            css_classes=["flat", "error"],
            tooltip_text=_("Delete"),
            valign=Gtk.Align.CENTER,
        )
        delete_btn.connect("clicked", self.delete)
        self.add_prefix(delete_btn)
        arrow = Gtk.Image(icon_name="errands-right-symbolic")
        arrow.bind_property(
            "visible",
            self,
            "activatable",
            GObject.BindingFlags.SYNC_CREATE | GObject.BindingFlags.BIDIRECTIONAL,
        )
        self.number_of_tasks: Gtk.Button = Gtk.Button(
            css_classes=["flat", "dim-label", "circular"],
            can_target=False,
            halign=Gtk.Align.CENTER,
            valign=Gtk.Align.CENTER,
        )
        self.add_suffix(self.number_of_tasks)

    def delete(self, _btn: Gtk.Button):
        for list in State.get_task_lists():
            for task in list.all_tasks:
                for tag in task.tags:
                    if tag.title == self.get_title():
                        task.tags_bar.remove(tag)
                        task.tags_bar_rev.set_reveal_child(
                            len(task.task_data.tags) - 1 > 0
                        )
                        break
        UserData.remove_tags([self.get_title()])
        self.tags.update_ui(False)

    def update_ui(self, tags_in_tasks: list[list[str]] = None):
        if not tags_in_tasks:
            tags_in_tasks: list[list[str]] = [t.tags for t in UserData.tasks if t.tags]

        count: int = 0
        for tags in tags_in_tasks:
            if self.get_title() in tags:
                count += 1

        self.set_activatable(False)
        self.number_of_tasks.set_label(str(count) if count > 0 else "")
