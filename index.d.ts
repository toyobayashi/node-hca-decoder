type decodeCallback = (err: Error | null, wavFilePath?: string) => void

export class HCADecoder {
  constructor(cipherKey1?: number, cipherKey2?: number)

  decodeToWaveFile (filenameHCA: string, callback?: decodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, callback?: decodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, callback?: decodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, mode: number, callback?: decodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, mode: number, loop: number, callback?: decodeCallback): void

  decodeToWaveFileSync (filenameHCA: string): boolean
  decodeToWaveFileSync (filenameHCA: string, filenameWAV: string): boolean
  decodeToWaveFileSync (filenameHCA: string, filenameWAV: string, volume: number): boolean
  decodeToWaveFileSync (filenameHCA: string, filenameWAV: string, volume: number, mode: number): boolean
  decodeToWaveFileSync (filenameHCA: string, filenameWAV: string, volume: number, mode: number, loop: number): boolean

  printInfo (filenameHCA: string): void
  decrypt (filenameHCA: string): boolean
}
