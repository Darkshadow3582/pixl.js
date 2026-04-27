#!/usr/bin/env python3
"""Apply the proposed Pixl.js issue triage actions.

The script is intentionally conservative:
- dry-run is the default;
- labels are created only when applying;
- issue actions that depend on an unmerged PR are skipped;
- managed comments include a marker so they are not posted twice.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from dataclasses import dataclass
from typing import Any


LABELS: dict[str, dict[str, str]] = {
    "support": {
        "color": "5319e7",
        "description": "Usage, update, recovery, BLE, or device-specific support",
    },
    "needs-info": {
        "color": "d4c5f9",
        "description": "More details are required before this can be acted on",
    },
    "blocked": {
        "color": "b60205",
        "description": "Blocked by release, hardware, external confirmation, or maintainer decision",
    },
    "release-needed": {
        "color": "fbca04",
        "description": "Fixed in code or PR, but not yet available in a release",
    },
    "area: docs": {
        "color": "0075ca",
        "description": "Documentation, guides, or issue templates",
    },
    "area: firmware": {
        "color": "0e8a16",
        "description": "Firmware behavior or device-side code",
    },
    "area: web": {
        "color": "1d76db",
        "description": "Web tools or browser-based update/file transfer flows",
    },
    "area: amiidb": {
        "color": "c2e0c6",
        "description": "Amiibo database, generated data, or Amiibo metadata",
    },
    "area: nfc": {
        "color": "bfd4f2",
        "description": "NFC scanning, emulation, or tag behavior",
    },
    "area: ui": {
        "color": "f9d0c4",
        "description": "Device UI, display, menus, and text rendering",
    },
    "area: hardware/vendor": {
        "color": "d93f0b",
        "description": "Hardware revisions, kits, or vendor-specific device variants",
    },
}


@dataclass(frozen=True)
class IssueAction:
    number: int
    labels: tuple[str, ...]
    comment: str | None = None
    close: bool = False
    state_reason: str = "completed"
    requires_pr: int | None = None

    @property
    def marker(self) -> str:
        return f"<!-- pixl-issue-triage:{self.number} -->"


ISSUE_ACTIONS: tuple[IssueAction, ...] = (
    IssueAction(
        number=225,
        labels=("duplicate",),
        close=True,
        state_reason="duplicate",
        comment="""This appears to be covered by #219, so I am closing this one as a duplicate to keep the discussion in one place.

The separate LFS-only directory rename bug found during investigation was handled in #439.""",
    ),
    IssueAction(
        number=267,
        labels=("support", "area: docs"),
        close=True,
        requires_pr=441,
        comment="""This should now be covered by the updated wrong-firmware recovery documentation in #441.

I am going to close this as answered/documented. If the issue is still reproducible on the latest firmware, please open a new bug report with the firmware version, LCD/OLED target, hardware/vendor, and reproduction steps.""",
    ),
    IssueAction(
        number=369,
        labels=("bug", "area: firmware", "release-needed"),
        close=True,
        comment="""This should be fixed by #436.

The firmware now recovers an invalid `settings.bin` by resetting it and saving a valid default file again, so settings such as language should persist normally after that recovery path runs.

I am going to close this as fixed. If it is still reproducible on the latest firmware, please open a new bug report with the firmware version, LCD/OLED target, hardware/vendor, and reproduction steps.""",
    ),
    IssueAction(
        number=400,
        labels=("documentation", "area: amiidb"),
        close=True,
        requires_pr=440,
        comment="""This should be covered by the Amiibo database source update in #440.

I am going to close this as fixed/documented. If the database source breaks again, please open a new issue with the failing source URL and generator output.""",
    ),
    IssueAction(
        number=408,
        labels=("enhancement", "area: amiidb", "area: ui"),
        close=True,
        requires_pr=437,
        comment="""This should be implemented by #437, which adds auto-scroll for long Amiibo descriptions.

I am going to close this as completed. If a specific description still does not scroll correctly on the latest firmware, please open a new bug report with the firmware version, LCD/OLED target, and the Amiibo entry name.""",
    ),
    IssueAction(
        number=422,
        labels=("bug", "area: nfc"),
        close=True,
        requires_pr=428,
        comment="""This should be fixed by #428, which hardens NFC buffer/page bounds during sequential scans.

I am going to close this as fixed. If it is still reproducible on the latest firmware, please open a new bug report with the firmware version, LCD/OLED target, and the exact scan sequence.""",
    ),
    IssueAction(
        number=423,
        labels=("support", "area: hardware/vendor"),
        close=True,
        comment="""This looks specific to a third-party/vendor firmware or device variant, so it is outside the current upstream firmware scope.

I am closing this as support/vendor-specific. If it is reproducible on the upstream Pixl.js firmware and supported hardware target, please open a new bug report with the firmware version, LCD/OLED target, hardware/vendor, and reproduction steps.""",
    ),
)


def run_gh(args: list[str], payload: dict[str, Any] | None = None) -> Any:
    cmd = ["gh", "api", *args]
    stdin = json.dumps(payload) if payload is not None else None
    result = subprocess.run(cmd, input=stdin, text=True, capture_output=True, check=False)
    if result.returncode != 0:
        raise RuntimeError(
            f"{' '.join(cmd)} failed with exit code {result.returncode}\n"
            f"stdout: {result.stdout.strip()}\n"
            f"stderr: {result.stderr.strip()}"
        )
    output = result.stdout.strip()
    return json.loads(output) if output else None


def gh_get(endpoint: str) -> Any:
    return run_gh([endpoint])


def gh_post(endpoint: str, payload: dict[str, Any]) -> Any:
    return run_gh(["-X", "POST", endpoint, "--input", "-"], payload)


def gh_patch(endpoint: str, payload: dict[str, Any]) -> Any:
    return run_gh(["-X", "PATCH", endpoint, "--input", "-"], payload)


def print_action(apply: bool, message: str) -> None:
    prefix = "APPLY" if apply else "DRY-RUN"
    print(f"[{prefix}] {message}")


def ensure_labels(repo: str, apply: bool) -> None:
    existing = {label["name"] for label in gh_get(f"repos/{repo}/labels?per_page=100")}
    for name, spec in LABELS.items():
        if name in existing:
            print_action(apply, f"label exists: {name}")
            continue

        print_action(apply, f"create label: {name}")
        if apply:
            gh_post(
                f"repos/{repo}/labels",
                {"name": name, "color": spec["color"], "description": spec["description"]},
            )


def is_pr_merged(repo: str, pr_number: int) -> bool:
    pr = gh_get(f"repos/{repo}/pulls/{pr_number}")
    return bool(pr.get("merged_at"))


def managed_comment_exists(repo: str, action: IssueAction) -> bool:
    comments = gh_get(f"repos/{repo}/issues/{action.number}/comments?per_page=100")
    return any(action.marker in comment.get("body", "") for comment in comments)


def apply_issue_action(repo: str, action: IssueAction, apply: bool, include_pending_prs: bool) -> None:
    if action.requires_pr is not None and not is_pr_merged(repo, action.requires_pr):
        message = f"skip #{action.number}: requires merged PR #{action.requires_pr}"
        if include_pending_prs:
            message += " (override enabled)"
        else:
            print_action(apply, message)
            return

    if action.labels:
        print_action(apply, f"add labels to #{action.number}: {', '.join(action.labels)}")
        if apply:
            gh_post(f"repos/{repo}/issues/{action.number}/labels", {"labels": list(action.labels)})

    if action.comment:
        body = f"{action.marker}\n{action.comment}"
        if managed_comment_exists(repo, action):
            print_action(apply, f"skip comment on #{action.number}: managed comment already exists")
        else:
            print_action(apply, f"comment on #{action.number}")
            if apply:
                gh_post(f"repos/{repo}/issues/{action.number}/comments", {"body": body})

    if action.close:
        print_action(apply, f"close #{action.number} as {action.state_reason}")
        if apply:
            gh_patch(
                f"repos/{repo}/issues/{action.number}",
                {"state": "closed", "state_reason": action.state_reason},
            )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", default="solosky/pixl.js", help="Repository in owner/name form")
    parser.add_argument("--apply", action="store_true", help="Apply changes. Default is dry-run.")
    parser.add_argument(
        "--only",
        choices=("all", "labels", "issues"),
        default="all",
        help="Limit the operation scope",
    )
    parser.add_argument(
        "--issue",
        action="append",
        type=int,
        default=[],
        help="Limit issue actions to one issue number. Can be repeated.",
    )
    parser.add_argument(
        "--include-pending-prs",
        action="store_true",
        help="Do not skip issue actions that depend on unmerged PRs",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    selected_issues = set(args.issue)

    try:
        if args.only in {"all", "labels"}:
            ensure_labels(args.repo, args.apply)

        if args.only in {"all", "issues"}:
            for action in ISSUE_ACTIONS:
                if selected_issues and action.number not in selected_issues:
                    continue
                apply_issue_action(args.repo, action, args.apply, args.include_pending_prs)
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    if not args.apply:
        print("\nDry-run complete. Re-run with --apply to mutate GitHub.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
