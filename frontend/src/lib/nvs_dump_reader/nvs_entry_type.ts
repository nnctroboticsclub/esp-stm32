export enum NVSType {
  U8 = 0x01,
  I8 = 0x11,
  U16 = 0x02,
  I16 = 0x12,
  U32 = 0x04,
  I32 = 0x14,
  U64 = 0x08,
  I64 = 0x18,
  STR = 0x21,
  BLOB = 0x42,
}

export type NVSTypeInstance<T extends NVSType> =
  T extends NVSType.U8 ? number :
  T extends NVSType.I8 ? number :
  T extends NVSType.U16 ? number :
  T extends NVSType.I16 ? number :
  T extends NVSType.U32 ? number :
  T extends NVSType.I32 ? number :
  T extends NVSType.U64 ? number :
  T extends NVSType.I64 ? number :
  T extends NVSType.STR ? string :
  T extends NVSType.BLOB ? number[] :
  never;