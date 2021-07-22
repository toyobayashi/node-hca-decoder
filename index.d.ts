/// <reference types="node" />

export declare type WavDecodeCallback = (err: Error | null, wavFilePath: string) => void
export declare type MemoryDecodeCallback = (err: Error | null, buffer: Buffer | null) => void

export declare interface HCAInfo {
  version: string
  channelCount: number
  samplingRate: number
  blockCount: number
  muteHeader: number
  muteFooter: number
  athType: number
  ciphType: number
  rvaVolume: number
  bitRate?: number
  blockSize?: number
  comp?: {
    comp1: number
    comp2: number
    comp3: number
    comp4: number
    comp5: number
    comp6: number
    comp7: number
    comp8: number
  }
  dec?: {
    dec1: number
    dec2: number
    dec3: number
    dec4: number
    dec5: number
    dec6: number
    dec7: number
  }
  vbr?: {
    vbr1: number
    vbr2: number
  }
  loop?: {
    start: number
    end: number
    count: number
    loop1: number
    infinite: boolean
  }
}

export declare class HCADecoder {
  static getInfo (filenameHCA: string): HCAInfo
  constructor(cipherKey1?: number, cipherKey2?: number)

  decodeToWaveFile (filenameHCA: string, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, mode: number, callback?: DecodeCallback): void
  decodeToWaveFile (filenameHCA: string, filenameWAV: string, volume: number, mode: number, loop: number, callback?: DecodeCallback): void

  decodeToWaveFileSync (filenameHCA: string, filenameWAV?: string, volume?: number, mode?: number, loop?: number): string

  decodeToMemory (filenameHCA: string, callback?: MemoryDecodeCallback): void
  decodeToMemory (filenameHCA: string, volume: number, callback?: MemoryDecodeCallback): void
  decodeToMemory (filenameHCA: string, volume: number, mode: number, callback?: MemoryDecodeCallback): void
  decodeToMemory (filenameHCA: string, volume: number, mode: number, loop: number, callback?: MemoryDecodeCallback): void

  decodeToMemorySync (filenameHCA: string, volume?: number, mode?: number, loop?: number): Buffer

  printInfo (filenameHCA: string): void
  // decrypt (filenameHCA: string): boolean
}
