import { useState, useCallback } from 'react'
import * as proto from '../lib/pixl.proto'

export interface FileEntry {
  name: string
  size: number
  type: 'REG' | 'DIR' | 'DRIVE'
  icon: string
  notes: string
  flags: { hide: boolean; readonly: boolean }
  amiibo: { head: number; tail: number }
}

export function useFiles() {
  const [files, setFiles] = useState<FileEntry[]>([])
  const [loading, setLoading] = useState(false)
  const [currentDir, setCurrentDir] = useState('')

  const reloadDrive = useCallback(async () => {
    setLoading(true)
    try {
      const res = await proto.vfs_get_drive_list()
      const drives = res.data.map((drive: any) => ({
        name: drive.label + ':/ [' + drive.name + ']',
        size: drive.status === 0
          ? formatSize(drive.used_size) + '/' + formatSize(drive.total_size)
          : '不可用: ' + drive.status,
        type: 'DRIVE' as const,
        icon: 'el-icon-box',
        notes: '',
        flags: { hide: false, readonly: false },
        amiibo: { head: 0, tail: 0 },
      }))
      setFiles(drives)
    } finally {
      setLoading(false)
    }
  }, [])

  const reloadFolder = useCallback(async (dir: string) => {
    setLoading(true)
    try {
      const res = await proto.vfs_read_folder(dir)
      if (res.status === 0) {
        const entries = res.data.map((file: any) => ({
          name: file.name,
          size: file.size,
          type: file.type === 0 ? 'REG' as const : 'DIR' as const,
          icon: file.type === 0 ? 'el-icon-document' : 'el-icon-folder',
          notes: file.meta.notes,
          flags: file.meta.flags,
          amiibo: file.meta.amiibo,
        }))
        setFiles(entries)
      }
    } finally {
      setLoading(false)
    }
  }, [])

  return { files, loading, currentDir, setCurrentDir, reloadDrive, reloadFolder, setFiles }
}

function formatSize(size: number): string {
  if (size < 1024) return size + ' B'
  if (size < 1024 * 1024) return (size / 1024).toFixed(2) + ' KB'
  return (size / 1024 / 1024).toFixed(2) + ' MB'
}
