---
name: unit-translation-sync
description: Check and sync translation keys for `Iocp20Coroutine/表/单位.xlsx`, ensuring every `名字Key` value in the `单位` worksheet exists in the `翻译` worksheet.
---

# Unit Translation Sync

Use the bundled script to compare the `单位` worksheet column `名字Key` against the `翻译` worksheet column `Key`.

## Quick Start

Run the checker first from the repository root:

```powershell
python .codex/skills/unit-translation-sync/scripts/sync_unit_translation.py --workbook "Iocp20Coroutine/表/单位.xlsx" --check
```

Append missing translation rows only when requested:

```powershell
python .codex/skills/unit-translation-sync/scripts/sync_unit_translation.py --workbook "Iocp20Coroutine/表/单位.xlsx" --apply
```

Use `--default-english` to initialize new English cells to a fixed value. The default is an empty string.

## Workflow

1. Read the workbook and locate the `单位` and `翻译` worksheets.
2. Find `名字Key` in `单位` and `Key/中文/英文` in `翻译`.
3. Compare non-empty, unique unit names with the existing translation keys.
4. Report missing names.
5. When applying, append rows with `Key` and `中文` set to the missing name and `英文` set to the requested default.
6. Preserve all unrelated workbook content.

## Rules

- Treat `Iocp20Coroutine/表/单位.xlsx` as the source of truth.
- Check before applying changes.
- Do not change or reorder existing translation rows.
- Do not hand-edit generated output files as part of this skill.

## Script

`scripts/sync_unit_translation.py`
