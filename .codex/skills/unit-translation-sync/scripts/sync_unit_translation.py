#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import re
import sys
import tempfile
import zipfile
from pathlib import Path
import xml.etree.ElementTree as ET

NS_MAIN = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
NS_REL = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
NS = {"a": NS_MAIN, "r": NS_REL}
ET.register_namespace("", NS_MAIN)


def ref_col_row(ref: str) -> tuple[str, int]:
    match = re.fullmatch(r"([A-Z]+)(\d+)", ref)
    if not match:
        raise ValueError(f"invalid cell reference: {ref}")
    return match.group(1), int(match.group(2))


def read_string_item(si: ET.Element) -> str:
    return "".join(node.text or "" for node in si.iterfind(".//a:t", NS))


class WorkbookEditor:
    def __init__(self, workbook_path: Path) -> None:
        self.workbook_path = workbook_path
        with zipfile.ZipFile(workbook_path, "r") as archive:
            self.files = {name: archive.read(name) for name in archive.namelist()}
        self.shared_strings_root = ET.fromstring(self.files["xl/sharedStrings.xml"])
        self.shared_strings = [
            read_string_item(item) for item in self.shared_strings_root.findall("a:si", NS)
        ]
        self.shared_string_index = {value: index for index, value in enumerate(self.shared_strings)}
        workbook_root = ET.fromstring(self.files["xl/workbook.xml"])
        rels_root = ET.fromstring(self.files["xl/_rels/workbook.xml.rels"])
        rel_map = {rel.attrib["Id"]: rel.attrib["Target"] for rel in rels_root}
        self.sheet_targets = {
            sheet.attrib["name"]: "xl/" + rel_map[sheet.attrib[f"{{{NS_REL}}}id"]]
            for sheet in workbook_root.find("a:sheets", NS)
        }

    def sheet_root(self, sheet_name: str) -> ET.Element:
        try:
            return ET.fromstring(self.files[self.sheet_targets[sheet_name]])
        except KeyError as exc:
            raise SystemExit(f"missing worksheet: {sheet_name}") from exc

    def cell_text(self, cell: ET.Element) -> str:
        if cell.attrib.get("t") == "s":
            value = cell.find("a:v", NS)
            return self.shared_strings[int(value.text)] if value is not None else ""
        if cell.attrib.get("t") == "inlineStr":
            inline = cell.find("a:is", NS)
            return "".join(node.text or "" for node in inline.iterfind(".//a:t", NS)) if inline is not None else ""
        value = cell.find("a:v", NS)
        return value.text or "" if value is not None else ""

    def rows_by_number(self, sheet_root: ET.Element) -> dict[int, ET.Element]:
        sheet_data = sheet_root.find("a:sheetData", NS)
        if sheet_data is None:
            raise SystemExit("worksheet missing sheetData")
        return {int(row.attrib["r"]): row for row in sheet_data.findall("a:row", NS)}

    def header_map(self, row: ET.Element) -> dict[str, str]:
        result = {}
        for cell in row.findall("a:c", NS):
            column, _ = ref_col_row(cell.attrib["r"])
            result[self.cell_text(cell).strip()] = column
        return result

    def collect_values(self, sheet_name: str, header_name: str) -> list[str]:
        rows = self.rows_by_number(self.sheet_root(sheet_name))
        header = self.header_map(rows[1])
        if header_name not in header:
            raise SystemExit(f"worksheet {sheet_name} missing header {header_name}")
        column = header[header_name]
        values = []
        seen = set()
        for row_number in sorted(rows):
            if row_number == 1:
                continue
            cell = next((item for item in rows[row_number].findall("a:c", NS) if ref_col_row(item.attrib["r"])[0] == column), None)
            value = self.cell_text(cell).strip() if cell is not None else ""
            if value and value not in seen:
                seen.add(value)
                values.append(value)
        return values

    def add_shared_string(self, value: str) -> int:
        index = self.shared_string_index.get(value)
        if index is None:
            index = len(self.shared_strings)
            self.shared_strings.append(value)
            self.shared_string_index[value] = index
            item = ET.SubElement(self.shared_strings_root, f"{{{NS_MAIN}}}si")
            text = ET.SubElement(item, f"{{{NS_MAIN}}}t")
            text.text = value
            self.shared_strings_root.attrib["uniqueCount"] = str(int(self.shared_strings_root.attrib.get("uniqueCount", "0")) + 1)
        self.shared_strings_root.attrib["count"] = str(int(self.shared_strings_root.attrib.get("count", "0")) + 1)
        return index

    def append_translation_rows(self, missing: list[str], default_english: str) -> None:
        root = self.sheet_root("翻译")
        rows = self.rows_by_number(root)
        header = self.header_map(rows[1])
        for required in ("Key", "中文", "英文"):
            if required not in header:
                raise SystemExit(f"worksheet 翻译 missing header {required}")
        sheet_data = root.find("a:sheetData", NS)
        last_row = max(rows)
        for offset, key in enumerate(missing, 1):
            row_number = last_row + offset
            row = ET.Element(f"{{{NS_MAIN}}}row", {"r": str(row_number)})
            values = {"Key": key, "中文": key, "英文": default_english}
            for name in ("Key", "中文", "英文"):
                column = header[name]
                cell = ET.SubElement(row, f"{{{NS_MAIN}}}c", {"r": f"{column}{row_number}", "t": "s"})
                node = ET.SubElement(cell, f"{{{NS_MAIN}}}v")
                node.text = str(self.add_shared_string(values[name]))
            sheet_data.append(row)
        self.files[self.sheet_targets["翻译"]] = ET.tostring(root, encoding="utf-8", xml_declaration=True)
        self.files["xl/sharedStrings.xml"] = ET.tostring(self.shared_strings_root, encoding="utf-8", xml_declaration=True)

    def write(self) -> None:
        descriptor, temp_name = tempfile.mkstemp(suffix=".xlsx", prefix="unit-translation-sync-")
        os.close(descriptor)
        Path(temp_name).unlink(missing_ok=True)
        try:
            with zipfile.ZipFile(temp_name, "w", zipfile.ZIP_DEFLATED) as archive:
                for name, content in self.files.items():
                    archive.writestr(name, content)
            Path(temp_name).replace(self.workbook_path)
        finally:
            Path(temp_name).unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Check or append missing unit translation rows.")
    parser.add_argument("--workbook", required=True)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--apply", action="store_true")
    parser.add_argument("--default-english", default="")
    args = parser.parse_args()
    path = Path(args.workbook)
    if not path.exists():
        print(f"workbook not found: {path}", file=sys.stderr)
        return 1
    editor = WorkbookEditor(path)
    unit_names = editor.collect_values("单位", "名字Key")
    translation_keys = set(editor.collect_values("翻译", "Key"))
    missing = [name for name in unit_names if name not in translation_keys]
    print(f"unit names: {len(unit_names)}")
    print(f"translation keys: {len(translation_keys)}")
    print(f"missing: {len(missing)}")
    for name in missing:
        print(name)
    if args.apply and missing:
        editor.append_translation_rows(missing, args.default_english)
        editor.write()
        print(f"appended rows: {len(missing)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
