# Web UI Redesign: Vue 2 → React with Modern Design

## Overview

Migrate the Pixl.js web interface from Vue 2 + Element UI to React + TypeScript + Tailwind CSS + shadcn/ui. Redesign the UX with a modern, clean tool aesthetic while preserving all existing functionality.

## Technical Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| Framework | React 18 + TypeScript | Requested by user |
| Build | Vite | Modern, fast, ESM-native |
| UI Kit | shadcn/ui (Radix primitives) | Clean, composable, minimal |
| Styling | Tailwind CSS 4 | Utility-first, consistent |
| Routing | React Router v6 | Standard for SPAs |
| i18n | react-i18next | Proven, parallel to current setup |
| BLE Layer | **No change** | Reuse `pixl.ble.js` and `pixl.proto.js` as-is |
| State | zustand (connection) + component state (file cache) | Lightweight, sufficient |

## Page Structure

### Layout

- **Sidebar** (fixed left): Icon + text nav items, collapsed state on narrow screens
- **Top bar**: Pixl.js logo (left), BLE connection button (center-right), language switcher (right)
  - BLE button shows: "Connect" (default), "Connecting..." (spinner), "Disconnect" (green, when connected)
- **Main content area**: Routes render here

### Routes

| Route | Page | Description |
|-------|------|-------------|
| `/` | Dashboard | Device overview |
| `/files` | File Manager | Browse & manage device files |
| `/settings` | Settings | Firmware update, language, about |

---

## Page: Dashboard (`/`)

Three-card layout in a responsive grid:

1. **Device Info Card**
   - Connection status indicator (green dot + "Connected" / gray dot + "Disconnected")
   - Firmware version string
   - BLE MAC address

2. **Storage Overview Card**
   - For each drive: ring/donut chart showing used vs total
   - Color coding: green (<60%), orange (60-85%), red (>85%)
   - Label: drive name + "X used / Y total"

3. **Quick Actions Card**
   - "Enter DFU Mode" button (with confirmation dialog)
   - "Refresh" button
   - "Disconnect" button (only when connected)

Empty state: When not connected, show a centered illustration/message: "Connect your Pixl.js device to get started" with a prompt to use the top-bar connect button.

---

## Page: File Manager (`/files`)

### Toolbar (top)
- Upload button → opens OS file picker
- New Folder button → inline prompt for folder name
- Delete button → batch delete selected items (with confirmation)
- Up button → navigate to parent directory
- Refresh button → re-read current directory
- Search input → filter files by name (client-side)
- View toggle → switch between **grid (icon)** and **table (list)** view

### File Display

- **Default: Grid view** — Large icon tiles arranged in a responsive grid
  - Folder: folder icon with name below
  - File: document icon with name, size, type
  - Click folder to enter, click file to download
  - Checkbox selection on hover

- **Alternative: List view** — Table columns: name (with icon), size, type, notes
  - Sortable columns
  - Row right-click context menu: Rename, Delete, Properties

### Path Navigation
- Breadcrumb trail below toolbar showing current path segments
- Click any segment to jump back

### Context Menu (both views)
- **Rename**: Inline prompt for new name
- **Delete**: Confirmation dialog, then delete
- **Properties**: Dialog showing file notes, hidden/readonly flags, Amiibo head/tail IDs

### Upload Dialog
- Drag-and-drop zone or file picker
- Progress bar per file (showing bytes written / total)
- Support multiple files
- Close dialog triggers: "Uploads in progress, cancel?" confirmation

---

## Page: Settings (`/settings`)

Vertical list of setting groups:

1. **Firmware**
   - Current version display
   - "Enter DFU Mode" button → confirmation → redirect to Web Bluetooth DFU page

2. **Language**
   - Dropdown/select with 8 languages: 简体中文, 繁體中文, English, Español, Русский, Deutsch, Svenska, 日本語
   - Persisted to cookie (same as current behavior)

3. **About**
   - Project name and version
   - GitHub link
   - Open-source license info

---

## BLE Integration

- **Unchanged**: `src/lib/pixl.ble.js` and `src/lib/pixl.proto.js` remain as-is
- Connection state managed via zustand store:
  ```ts
  interface ConnectionState {
    connected: boolean
    version: string | null
    bleAddress: string | null
    connect: () => Promise<void>
    disconnect: () => void
  }
  ```
- Event dispatcher pattern preserved (shared event bus pattern)
- File operations wrapped in async hooks (e.g., `useFiles`, `useDriveInfo`)

---

## i18n Strategy

- Migrate existing locale files (zh_Hans, en_US, etc.) to react-i18next JSON format
- Same key structure, same translation content
- Language persistence via cookie (react-cookie)
- Default: browser language detection → fallback to zh_CN

---

## State Management

| State | Location | Details |
|-------|----------|---------|
| BLE connection | zustand store | Globally accessible |
| Current directory | component state (`useFiles` hook) | Per-route |
| File list cache | component state (`useFiles` hook) | Cleared on disconnect |
| Selected files | component state | For batch operations |
| View mode preference | localStorage | Persist grid/list choice |
| Language preference | cookie | Persisted across sessions |
| UI theme (sidebar collapsed) | component state | Session-only |

---

## Error Handling

- BLE connection failures: toast notification at top-right
- File operation failures: inline error toast + keep file list unchanged
- Upload errors: per-file error status in upload queue
- Network/Bluetooth disconnection: auto-detect, reset file view, show toast
- DFU entry: confirmation dialog, then redirect

---

## What Stays the Same

- All BLE protocol logic (pixl.ble.js, pixl.proto.js)
- All i18n translation strings (just format changes)
- All feature behavior (upload, download, rename, delete, format, meta edit)
- Device communication protocol (command IDs, frame format, chunking)
- Deployment flow (build → copy to gh-pages → git subtree push)

---

## What Changes

- Framework: Vue 2 → React 18 + TypeScript
- Build tool: Webpack 2 → Vite
- UI: Element UI → shadcn/ui + Tailwind CSS
- Layout: Single monolithic component → routed multi-page with sidebar
- File view: Table-only → Grid (default) + List (toggle)
- Visual: Traditional Chinese enterprise style → Clean modern minimal
