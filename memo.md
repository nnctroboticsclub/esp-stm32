# めも

## やること

### data-proxy/link でチェックサムを追加

```plain
<<Send>>

0:
Send --> Recv: packet
Send --> Recv: Checksum 16bit
Send <-- Recv: / 00 [OK]
Send <-- Recv: \ !0 [NG]

if NG:
  goto 0
```

↑こんな

## シリアル通信を UDP に流すやつ

ブロードキャストで 9088 に次の形式で投げる

| Offset | Size   | Description  |
| ------ | ------ | ------------ |
| 0x00   | 1      | Version (00) |
| 0x01   | 1      | Command      |
| 0x02   | 2      | Id           |
| 0x04   | 4      | length       |
| 0x08   | length | data         |

### Command

| Value | Description  |
| ----- | ------------ |
| 0x00  | Send         |
| 0x01  | Link Name    |
| ^     | Data is name |

## Setting.svelte でアクティブプロファイルの設定
