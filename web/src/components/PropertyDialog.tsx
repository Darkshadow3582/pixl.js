import { useState } from 'react'
import { FileEntry } from '../hooks/useFiles'
import * as proto from '../lib/pixl.proto'

interface Props {
  file: FileEntry | null
  currentDir: string
  open: boolean
  onClose: () => void
}

export default function PropertyDialog({ file, currentDir, open, onClose }: Props) {
  const [notes, setNotes] = useState(file?.notes || '')
  const [hide, setHide] = useState(file?.flags.hide || false)
  const [readonly, setReadonly] = useState(file?.flags.readonly || false)

  if (!open || !file) return null

  const handleSave = async () => {
    const path = currentDir + '/' + file.name
    await proto.vfs_update_meta(path, {
      notes,
      flags: { hide, readonly },
      amiibo: file.amiibo,
    })
    onClose()
  }

  return (
    <div className="fixed inset-0 bg-black/50 z-50 flex items-center justify-center">
      <div className="bg-white rounded-lg shadow-lg w-96 p-6">
        <div className="flex justify-between items-center mb-4">
          <h3 className="font-medium">属性 - {file.name}</h3>
          <button onClick={onClose} className="text-muted-foreground hover:text-foreground">✕</button>
        </div>
        <div className="space-y-4">
          <div>
            <label className="text-sm font-medium block mb-1">备注</label>
            <input
              value={notes}
              onChange={(e) => setNotes(e.target.value)}
              className="w-full h-9 rounded-md border border-input bg-background px-3 text-sm"
            />
          </div>
          <div className="flex gap-4">
            <label className="flex items-center gap-2 text-sm">
              <input type="checkbox" checked={hide} onChange={(e) => setHide(e.target.checked)} />
              隐藏
            </label>
            <label className="flex items-center gap-2 text-sm">
              <input type="checkbox" checked={readonly} onChange={(e) => setReadonly(e.target.checked)} />
              只读
            </label>
          </div>
          <div className="text-xs text-muted-foreground">
            <p>Amiibo Head: 0x{file.amiibo.head.toString(16).padStart(8, '0')}</p>
            <p>Amiibo Tail: 0x{file.amiibo.tail.toString(16).padStart(8, '0')}</p>
          </div>
        </div>
        <div className="flex justify-end gap-2 mt-4">
          <button onClick={onClose} className="px-4 py-2 rounded-md border text-sm">取消</button>
          <button onClick={handleSave} className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm">保存</button>
        </div>
      </div>
    </div>
  )
}
