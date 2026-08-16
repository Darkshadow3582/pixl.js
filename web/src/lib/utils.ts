export function formatSize(size: number): string {
  if (size < 1024) {
    return size + ' B'
  } else if (size < 1024 * 1024) {
    return (size / 1024).toFixed(2) + ' KB'
  } else {
    return (size / 1024 / 1024).toFixed(2) + ' MB'
  }
}

export function appendSegment(dir: string, seg: string): string {
  const drive = dir.substring(0, 2)
  let path = dir.substring(2)
  if (path === '/') {
    return dir + seg
  }
  return dir + '/' + seg
}
