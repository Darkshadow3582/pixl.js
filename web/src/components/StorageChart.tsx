import { PieChart, Pie, Cell, ResponsiveContainer } from 'recharts'

interface Props {
  label: string
  used: number
  total: number
  status: number
}

export default function StorageChart({ label, used, total }: Props) {
  const usedPercent = total > 0 ? (used / total) * 100 : 0
  const color = usedPercent < 60 ? '#22c55e' : usedPercent < 85 ? '#f97316' : '#ef4444'
  const data = [
    { name: '已用', value: used },
    { name: '剩余', value: Math.max(0, total - used) },
  ]

  return (
    <div className="rounded-lg border bg-card p-5">
      <h3 className="font-medium text-sm mb-3">存储 - {label}</h3>
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
          <p className="text-muted-foreground">已用 {formatSize(used)}</p>
          <p className="text-muted-foreground">总计 {formatSize(total)}</p>
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
