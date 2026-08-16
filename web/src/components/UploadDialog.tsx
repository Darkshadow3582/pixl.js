import { useState, useCallback } from 'react'
import { useTranslation } from 'react-i18next'
import * as proto from '../lib/pixl.proto'

interface Props {
  currentDir: string
  open: boolean
  onClose: () => void
  onDone: () => void
}

export default function UploadDialog({ currentDir, open, onClose, onDone }: Props) {
  const { t } = useTranslation()
  const [files, setFiles] = useState<File[]>([])
  const [uploading, setUploading] = useState(false)
  const [progress, setProgress] = useState<Record<string, number>>({})

  const handleFileChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files) {
      setFiles(Array.from(e.target.files))
    }
  }, [])

  const handleUpload = useCallback(async () => {
    setUploading(true)
    for (const file of files) {
      await new Promise<void>((resolve) => {
        proto.vfs_helper_write_file(
          currentDir + '/' + file.name,
          file,
          (p: any) => setProgress(prev => ({ ...prev, [file.name]: p.written_bytes / p.total_bytes * 100 })),
          () => resolve(),
          () => resolve(),
        )
      })
    }
    setUploading(false)
    onDone()
    onClose()
  }, [files, currentDir, onClose, onDone])

  if (!open) return null

  return (
    <div className="fixed inset-0 bg-black/50 z-50 flex items-center justify-center">
      <div className="bg-white rounded-lg shadow-lg w-96 p-6">
        <div className="flex justify-between items-center mb-4">
          <h3 className="font-medium">{t('upload.title')}</h3>
          <button onClick={onClose} className="text-muted-foreground hover:text-foreground">✕</button>
        </div>
        <div className="border-2 border-dashed rounded-lg p-6 text-center mb-4">
          <input
            type="file"
            multiple
            onChange={handleFileChange}
            className="w-full"
          />
          <p className="text-sm text-muted-foreground mt-2">{t('upload.drag')}{t('upload.click')}</p>
        </div>
        {files.length > 0 && (
          <div className="space-y-2 mb-4 max-h-40 overflow-auto">
            {files.map((f) => (
              <div key={f.name} className="text-sm">
                <div className="flex justify-between">
                  <span className="truncate">{f.name}</span>
                  <span className="text-muted-foreground">
                    {progress[f.name] !== undefined ? Math.round(progress[f.name]) + '%' : t('dialog.waiting')}
                  </span>
                </div>
                {progress[f.name] !== undefined && (
                  <div className="w-full h-1 bg-muted rounded-full mt-1">
                    <div className="h-full bg-primary rounded-full" style={{ width: progress[f.name] + '%' }} />
                  </div>
                )}
              </div>
            ))}
          </div>
        )}
        <div className="flex justify-end gap-2">
          <button onClick={onClose} className="px-4 py-2 rounded-md border text-sm">{t('dialog.cancel')}</button>
          <button
            onClick={handleUpload}
            disabled={uploading || files.length === 0}
            className="px-4 py-2 rounded-md bg-primary text-primary-foreground text-sm disabled:opacity-50"
          >
            {uploading ? t('dialog.uploading') : t('dialog.start_upload')}
          </button>
        </div>
      </div>
    </div>
  )
}
