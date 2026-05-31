import { useTranslation } from 'react-i18next'
import { PieChart, Pie, Cell, ResponsiveContainer } from 'recharts'

interface Props {
  label: string
  used: number
  total: number
  status: number
}

export default function StorageChart({ label, used, total }: Props) {
  const { t } = useTranslation()
  const free = used
  const usedVal = Math.max(0, total - free)
  const usedPercent = total > 0 ? (usedVal / total) * 100 : 0
  const color = usedPercent < 60 ? '#22c55e' : usedPercent < 85 ? '#f97316' : '#ef4444'
  const data = [
    { name: t('dashboard.used'), value: usedVal },
    { name: t('dashboard.free'), value: free },
  ]

  return (
    <div className="rounded-lg border bg-card p-5">
      <h3 className="font-medium text-sm mb-3">{t('dashboard.storage', { label })}</h3>
      <div className="flex items-center gap-4">
        <div className="w-20 h-20">
          <ResponsiveContainer width="100%" height="100%">
            <PieChart>
              <Pie
                data={data}
                cx="50%"
                cy="50%"
                innerRadius={24}
                outerRadius={36}
                startAngle={90}
                endAngle={-270}
                dataKey="value"
              >
                <Cell fill={color} />
                <Cell fill="#f1f5f9" />
              </Pie>
            </PieChart>
          </ResponsiveContainer>
        </div>
        <div className="text-sm">
          <p className="text-muted-foreground">{t('dashboard.used')} {formatSize(usedVal)}</p>
          <p className="text-muted-foreground">{t('dashboard.total')} {formatSize(total)}</p>
        </div>
      </div>
    </div>
  )
}

function formatSize(size: number): string {
  if (size < 1024) return size + ' B'
  if (size < 1024 * 1024) return (size / 1024).toFixed(1) + ' KB'
  return (size / 1024 / 1024).toFixed(1) + ' MB'
}
