import { useEffect, useRef } from 'react'

interface DialogProps {
  open: boolean
  title: string
  children: React.ReactNode
  onClose: () => void
  footer?: React.ReactNode
}

export default function Dialog({ open, title, children, onClose, footer }: DialogProps) {
  const ref = useRef<HTMLDivElement>(null)

  useEffect(() => {
    if (!open) return
    const handleKey = (e: KeyboardEvent) => {
      if (e.key === 'Escape') onClose()
    }
    document.addEventListener('keydown', handleKey)
    return () => document.removeEventListener('keydown', handleKey)
  }, [open, onClose])

  if (!open) return null

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/40" onClick={() => onClose()}>
      <div ref={ref} className="bg-white rounded-lg shadow-lg w-96 max-w-full mx-4" onClick={e => e.stopPropagation()}>
        <div className="px-6 py-4 border-b">
          <h3 className="font-medium">{title}</h3>
        </div>
        <div className="px-6 py-4">{children}</div>
        {footer && <div className="px-6 py-3 border-t flex justify-end gap-2">{footer}</div>}
      </div>
    </div>
  )
}
