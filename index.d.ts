export declare type DecodeCallback = (err: Error | null, wavFilePath: string) => void

export declare class HCADecoder {
  constructor(cipherKey1?: number, cipherKey2?: number)

  decodeToWaveFile (filenameHCA: string, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, mode: number, callback?: DecodeCallback): void

  decodeToWaveFileSync (filenameHCA: string, filenameWAV?: string, volume?: number, mode?: number): string

  printInfo (filenameHCA: string): void
  // decrypt (filenameHCA: string): boolean
}
