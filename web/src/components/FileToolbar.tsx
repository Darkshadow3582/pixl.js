import { FolderPlus, Upload, Trash2, ArrowUp, RefreshCw, Search, LayoutGrid, List } from 'lucide-react'

interface Props {
  viewMode: 'grid' | 'list'
  onToggleView: () => void
  onUpload: () => void
  onNewFolder: () => void
  onDelete: () => void
  onUp: () => void
  onRefresh: () => void
  onSearch: (query: string) => void
  hasSelection: boolean
}

export default function FileToolbar({
  viewMode, onToggleView, onUpload, onNewFolder, onDelete,
  onUp, onRefresh, onSearch, hasSelection,
}: Props) {
  return (
    <div className="flex items-center gap-2 pb-4 border-b mb-4">
      <button onClick={onUpload} className="btn-toolbar" title="上传">
        <Upload className="w-4 h-4" /> 上传
      </button>
      <button onClick={onNewFolder} className="btn-toolbar" title="新建文件夹">
        <FolderPlus className="w-4 h-4" /> 新建
      </button>
      <button onClick={onDelete} className="btn-toolbar text-destructive" title="删除" disabled={!hasSelection}>
        <Trash2 className="w-4 h-4" /> 删除
      </button>
      <div className="w-px h-6 bg-border mx-1" />
      <button onClick={onUp} className="btn-toolbar" title="返回上级">
        <ArrowUp className="w-4 h-4" />
      </button>
      <button onClick={onRefresh} className="btn-toolbar" title="刷新">
        <RefreshCw className="w-4 h-4" />
      </button>
      <div className="flex-1" />
      <div className="relative">
        <Search className="w-4 h-4 absolute left-3 top-1/2 -translate-y-1/2 text-muted-foreground" />
        <input
          placeholder="搜索文件..."
          onChange={(e) => onSearch(e.target.value)}
          className="h-9 w-48 rounded-md border border-input bg-background pl-9 pr-3 text-sm"
        />
      </div>
      <button onClick={onToggleView} className="btn-toolbar" title={viewMode === 'grid' ? '列表视图' : '网格视图'}>
        {viewMode === 'grid' ? <List className="w-4 h-4" /> : <LayoutGrid className="w-4 h-4" />}
      </button>
      <style>{`
  .btn-toolbar {
    display: inline-flex;
    align-items: center;
    gap: 0.375rem;
    padding: 0.375rem 0.75rem;
    font-size: 0.875rem;
    border-radius: 0.375rem;
    border: 1px solid hsl(var(--border));
    background: hsl(var(--background));
    color: hsl(var(--foreground));
    cursor: pointer;
  }
  .btn-toolbar:hover { background: hsl(var(--accent)); }
  .btn-toolbar:disabled { opacity: 0.5; cursor: not-allowed; }
`}</style>
    </div>
  )
}
