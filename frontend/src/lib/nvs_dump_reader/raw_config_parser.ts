export class RawConfigParser {
  constructor(data: number[]) {
    if (!data) {
      throw new Error("Data is null");
    }
    this.data = data;
  }

  ReadUint8(): number {
    const value = this.data[0];
    this.data = this.data.slice(1);
    return value >>> 0;
  }

  ReadUint16():number {
    const value = (this.data[0] << 8) | (this.data[1]);
    this.data = this.data.slice(2);
    return value >>> 0;
  }

  ReadUint32():number {
    const value = (this.data[0] << 24) | (this.data[1] << 16) | (this.data[2] << 8) | this.data[3];
    this.data = this.data.slice(4);
    return value >>> 0;
  }

  ReadUint64():number {
    const value = (this.data[0] << 56) | (this.data[1] << 48) | (this.data[2] << 40) | (this.data[3] << 32) | (this.data[4] << 24) | (this.data[5] << 16) | (this.data[6] << 8) | this.data[7];
    this.data = this.data.slice(8);
    return value >>> 0;
  }

  ReadData(len: number):number[] {
    const value = this.data.slice(0, len);
    this.data = this.data.slice(len);
    return value;
  }

  ReadString(len: number): string {
    return String.fromCharCode.apply(null, this.ReadData(len)).replace(/\0/g, '');
  }

  DataAvailable(): boolean {
    return this.data.length > 0;
  }

}