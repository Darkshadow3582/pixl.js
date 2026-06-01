import { useEffect, useRef } from 'react'
import { useTranslation } from 'react-i18next'
import { Pencil, Trash2, Info, Download } from 'lucide-react'

interface Props {
  x: number
  y: number
  onRename: () => void
  onDelete: () => void
  onDownload?: () => void
  onProperties: () => void
  onClose: () => void
}

export default function FileContextMenu({ x, y, onRename, onDelete, onDownload, onProperties, onClose }: Props) {
  const { t } = useTranslation()
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
    { icon: Pencil, label: t('contxmenu.rename'), action: onRename },
    ...(onDownload ? [{ icon: Download, label: t('contxmenu.download'), action: onDownload }] : []),
    { icon: Trash2, label: t('contxmenu.del'), action: onDelete, danger: true as const },
    { icon: Info, label: t('contxmenu.prop'), action: onProperties },
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
