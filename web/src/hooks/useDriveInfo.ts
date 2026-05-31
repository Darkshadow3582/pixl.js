import { useState, useCallback } from 'react'
import * as proto from '../lib/pixl.proto'

export interface DriveInfo {
  label: string
  name: string
  totalSize: number
  freeSize: number
  status: number
}

export function useDriveInfo() {
  const [drives, setDrives] = useState<DriveInfo[]>([])
  const [loading, setLoading] = useState(false)

  const fetchDrives = useCallback(async () => {
    setLoading(true)
    try {
      const res = await proto.vfs_get_drive_list()
      setDrives(res.data.map((d: any) => ({
        label: d.label,
        name: d.name,
        totalSize: d.total_size,
        freeSize: d.used_size,
        status: d.status,
      })))
    } finally {
      setLoading(false)
    }
  }, [])

  return { drives, loading, fetchDrives }
}
