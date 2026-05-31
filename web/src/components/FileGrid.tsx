import { FileEntry } from '../hooks/useFiles'
import { Folder, File } from 'lucide-react'

interface Props {
  files: FileEntry[]
  selected: Set<string>
  onSelect: (name: string) => void
  onOpen: (file: FileEntry) => void
  onContextMenu: (file: FileEntry, x: number, y: number) => void
}

export default function FileGrid({ files, selected, onSelect, onOpen, onContextMenu }: Props) {
  return (
    <div className="grid grid-cols-5 sm:grid-cols-6 md:grid-cols-8 lg:grid-cols-10 gap-2">
      {files.map((file) => (
        <div
          key={file.name}
          className={`relative flex flex-col items-center gap-1 p-3 rounded-lg border cursor-pointer hover:bg-accent ${
            selected.has(file.name) ? 'border-primary bg-primary/5' : ''
          }`}
          onClick={() => onOpen(file)}
          onContextMenu={(e) => {
            e.preventDefault()
            onContextMenu(file, e.clientX, e.clientY)
          }}
        >
          <input
            type="checkbox"
            checked={selected.has(file.name)}
            onChange={() => onSelect(file.name)}
            className="absolute top-1 right-1"
            onClick={(e) => e.stopPropagation()}
          />
          {file.type === 'DIR' ? (
            <Folder className="w-8 h-8 text-amber-500" />
          ) : (
            <File className="w-8 h-8 text-blue-500" />
          )}
          <span className="text-xs text-center truncate w-full">{file.name}</span>
          {file.type === 'REG' && (
            <span className="text-xs text-muted-foreground">
              {formatSize(file.size)}
            </span>
          )}
        </div>
      ))}
    </div>
  )
}

function formatSize(size: number): string {
  if (size < 1024) return size + ' B'
  if (size < 1024 * 1024) return (size / 1024).toFixed(1) + ' KB'
  return (size / 1024 / 1024).toFixed(1) + ' MB'
}
