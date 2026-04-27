# Issue Triage Proposal

This document proposes a lightweight issue triage workflow for maintainers. The
goal is to close answered issues with clear comments, keep open issues easier to
sort, and avoid labels that duplicate GitHub's built-in PR linking.

## Labels

Use the existing default labels where possible:

- `bug`
- `documentation`
- `duplicate`
- `enhancement`
- `good first issue`
- `help wanted`
- `invalid`
- `question`

Suggested additional labels:

- `support`: usage, update, recovery, BLE, or device-specific help.
- `needs-info`: missing firmware version, LCD/OLED target, hardware/vendor,
  reproduction steps, logs, or photos.
- `blocked`: waiting on hardware, release, maintainer decision, or external
  confirmation.
- `release-needed`: fixed in code or a pull request, but not yet shipped in a
  release.
- `area: docs`
- `area: firmware`
- `area: web`
- `area: amiidb`
- `area: nfc`
- `area: ui`
- `area: hardware/vendor`

`has-pr` is intentionally not included. GitHub already links issues and pull
requests through references, closing keywords, and timeline events.

`wontfix` should only be used when maintainers explicitly decide that something
will not be supported. For normal triage, prefer `support`, `duplicate`,
`invalid`, `needs-info`, or a clear closing comment.

## Closing Policy

Before closing an issue, leave a short comment explaining why it is being closed
and what the user should provide if the problem still exists.

Suggested comment for issues resolved by documentation or code:

```markdown
This should now be covered by the updated documentation in #PR.

I am going to close this as answered/documented. If the issue is still
reproducible on the latest firmware, please open a new bug report with the
firmware version, LCD/OLED target, hardware/vendor, and reproduction steps.
```

Suggested comment for duplicates:

```markdown
This appears to be covered by #CANONICAL_ISSUE / #PR, so I am closing this one
as duplicate to keep the discussion in one place.
```

Suggested comment for support/vendor-specific cases:

```markdown
This looks specific to a third-party/vendor firmware or device variant, so it is
outside the current upstream firmware scope. Closing as support/vendor-specific.
```

## Current Triage Waves

Wave 1 closes issues already covered by merged or open pull requests:

| Issue | Action |
| --- | --- |
| #225 | Close as duplicate/already answered by #219; #439 fixed a separate LFS-only directory rename bug found during investigation. |
| #267 | Close after #441 merges; wrong LCD/OLED firmware recovery is documented there. |
| #369 | Close as fixed by #436; invalid `settings.bin` can now be recovered and saved again. |
| #400 | Close after #440 merges if the Amiibo DB source change is accepted. |
| #408 | Close after #437 merges; long Amiibo description auto-scroll is implemented there. |
| #422 | Close after #428 merges; sequential NFC scan freeze hardening is implemented there. |
| #423 | Close as support/vendor-specific if maintainers agree this device variant is outside upstream scope. |

Wave 2 should be handled by small documentation pull requests:

1. Firmware update and recovery troubleshooting.
2. Card emulator, files, dumps, and backup behavior.
3. README project identity and Amiibo DB source clarification.
4. Issue templates for bug reports, support, feature requests, and documentation.

## Helper Script

Maintainers can preview the proposed label and issue actions with:

```sh
python3 .github/scripts/apply_issue_triage.py --repo solosky/pixl.js
```

Apply the actions with:

```sh
python3 .github/scripts/apply_issue_triage.py --repo solosky/pixl.js --apply
```

The script is dry-run by default, skips actions that depend on unmerged pull
requests, and avoids posting the same managed comment twice.
