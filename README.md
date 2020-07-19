# DNS request tool

A simple CLI tool to resolve domain names to its IPs

By default it will use Cloudflare's 1.1.1.1

<i>Disclaimer: Built for educational purposes and for fun</i>

## Usage
```bash
./dns ycombinator.com
```

```bash
DNS Server: 1.1.1.1

- Query
Name: ycombinator.com
Answers RRs: 4
Authority RRs: 0
Additional RRs: 0

- Answer(1)
Type: 1
Class: 1
TTL: 60
Data length: 4
Address: 99.86.109.24


- Answer(2)
Type: 1
Class: 1
TTL: 60
Data length: 4
Address: 99.86.109.94


- Answer(3)
Type: 1
Class: 1
TTL: 60
Data length: 4
Address: 99.86.109.39


- Answer(4)
Type: 1
Class: 1
TTL: 60
Data length: 4
Address: 99.86.109.58 
```