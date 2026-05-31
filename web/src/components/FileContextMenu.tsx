import { useEffect, useRef } from 'react'
import { Pencil, Trash2, Info } from 'lucide-react'

interface Props {
  x: number
  y: number
  onRename: () => void
  onDelete: () => void
  onProperties: () => void
  onClose: () => void
}

export default function FileContextMenu({ x, y, onRename, onDelete, onProperties, onClose }: Props) {
  const ref = useRef<HTMLDivElement>(null)

  useEffect(() => {
    const handleClick = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) {
        onClose()
      }
    }
    document.addEventListener('mousedown', handleClick)
    return () => document.removeEventListener('mousedown', handleClick)
  }, [onClose])

  const items = [
    { icon: Pencil, label: '重命名', action: onRename },
    { icon: Trash2, label: '删除', action: onDelete, danger: true as const },
    { icon: Info, label: '属性', action: onProperties },
  ]

  return (
    <div
      ref={ref}
      className="fixed z-50 w-40 rounded-lg border bg-card shadow-lg py-1"
      style={{ left: x, top: y }}
    >
      {items.map((item) => (
        <button
          key={item.label}
          onClick={() => { item.action(); onClose() }}
          className={`w-full flex items-center gap-2 px-3 py-1.5 text-sm hover:bg-accent ${
            item.danger ? 'text-destructive' : ''
          }`}
        >
          <item.icon className="w-4 h-4" />
          {item.label}
        </button>
      ))}
    </div>
  )
}
