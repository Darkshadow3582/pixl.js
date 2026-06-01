import { CheckCircle2, XCircle } from 'lucide-react'

export interface DownloadItem {
  id: string
  name: string
  progress: number  // 0-1, 1=done, -1=error
}

interface Props {
  downloads: DownloadItem[]
}

export default function DownloadToast({ downloads }: Props) {
  if (downloads.length === 0) return null

  return (
    <div className="fixed top-4 right-4 z-50 space-y-2">
      {downloads.map((d) => (
        <div key={d.id} className="bg-white rounded-lg border shadow-lg p-4 w-72">
          <div className="flex items-center justify-between mb-2">
            <p className="text-sm truncate flex-1">{d.name}</p>
            {d.progress >= 1 && <CheckCircle2 className="w-4 h-4 text-green-500 shrink-0 ml-2" />}
            {d.progress <= -1 && <XCircle className="w-4 h-4 text-red-500 shrink-0 ml-2" />}
            {d.progress >= 0 && d.progress < 1 && (
              <span className="text-xs text-muted-foreground shrink-0 ml-2">{Math.round(d.progress * 100)}%</span>
            )}
          </div>
          <div className="w-full h-1.5 bg-muted rounded-full overflow-hidden">
            <div
              className={`h-full rounded-full transition-all duration-300 ${
                d.progress <= -1 ? 'bg-destructive' : 'bg-primary'
              }`}
              style={{ width: `${d.progress <= -1 ? 100 : Math.max(d.progress * 100, 4)}%` }}
            />
          </div>
        </div>
      ))}
    </div>
  )
}
