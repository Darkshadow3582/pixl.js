import { FileEntry } from '../hooks/useFiles'
import { Folder, File } from 'lucide-react'

interface Props {
  files: FileEntry[]
  selected: Set<string>
  onSelect: (name: string) => void
  onOpen: (file: FileEntry) => void
  onContextMenu: (file: FileEntry, x: number, y: number) => void
}

export default function FileList({ files, selected, onSelect, onOpen, onContextMenu }: Props) {
  return (
    <table className="w-full text-sm">
      <thead>
        <tr className="border-b text-muted-foreground">
          <th className="w-8 p-2"></th>
          <th className="text-left p-2 font-medium">名称</th>
          <th className="text-left p-2 font-medium w-24">大小</th>
          <th className="text-left p-2 font-medium w-20">类型</th>
          <th className="text-left p-2 font-medium">备注</th>
        </tr>
      </thead>
      <tbody>
        {files.map((file) => (
          <tr
            key={file.name}
            className={`border-b hover:bg-accent cursor-pointer ${
              selected.has(file.name) ? 'bg-primary/5' : ''
            }`}
            onClick={() => onOpen(file)}
            onContextMenu={(e) => {
              e.preventDefault()
              onContextMenu(file, e.clientX, e.clientY)
            }}
          >
            <td className="p-2" onClick={(e) => e.stopPropagation()}>
              <input
                type="checkbox"
                checked={selected.has(file.name)}
                onChange={() => onSelect(file.name)}
              />
            </td>
            <td className="p-2 flex items-center gap-2">
              {file.type === 'DIR' ? (
                <Folder className="w-4 h-4 text-amber-500" />
              ) : (
                <File className="w-4 h-4 text-blue-500" />
              )}
              {file.name}
            </td>
            <td className="p-2 text-muted-foreground">{formatDisplaySize(file)}</td>
            <td className="p-2 text-muted-foreground">{file.type === 'DIR' ? '文件夹' : '文件'}</td>
            <td className="p-2 text-muted-foreground truncate max-w-48">{file.notes}</td>
          </tr>
        ))}
      </tbody>
    </table>
  )
}

function formatDisplaySize(file: FileEntry): string {
  if (file.type !== 'REG') return '-'
  if (file.size < 1024) return file.size + ' B'
  if (file.size < 1024 * 1024) return (file.size / 1024).toFixed(1) + ' KB'
  return (file.size / 1024 / 1024).toFixed(1) + ' MB'
}
