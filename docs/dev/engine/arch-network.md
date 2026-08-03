# Networking

The client and the server talk over ENet, a thin reliability layer on top of UDP. Each side reads and writes [buffers](arch-core.md#buffers), and every packet starts with a 16-bit unsigned integer that names its ID.

A client goes through one of two exchanges after it connects: a status ping, or a full login. Both start the same way and both end with the server closing or keeping the connection.

## Status pings

A client can connect just to ask a server about itself: protocol version, MOTD, player count, and slot limit. The server does not keep this connection open.

### Flow

|Step|Sender|Receiver|Description|
|----|----|----|----|
|1|Client|Server|The client connects|
|2|Client|Server|The client sends a status request packet|
|3|Server|Client|The server responds with a status response packet|
|4|Server|Client|The server closes the connection|

## Authentication

If the client sends a login request instead of a status request, the server starts an authentication exchange.

### Flow

|Step|Sender|Receiver|Description|
|----|----|----|----|
|1|Client|Server|The client connects|
|2|Client|Server|The client sends a login request packet: username, public Ed25519 key, invite code, protocol version, and registry hashes|
|3|Server|Client|If the server accepts the request, it responds with a challenge packet holding a random nonce|
|4|Client|Server|The client signs the nonce, the server's password, and the current UTC time, and sends the signature back in a challenge response packet|
|5|Server|Client|If the signature matches, the server admits the client with a packet holding its assigned username|
|6|N/A|N/A|Entity exchange and game packets begin here|

##### Identity

The client proves its identity with an Ed25519 keypair. The server challenges the client with a random nonce, and the client signs it.

##### Server passwords

A server-side password, if the server has one, is folded into the signed message. This way, the exchange also proves the client knows the password, without ever sending it.

##### Invite codes

The invite code is a one-time code. On a successful login, the server consumes the invite code and adds the client's public key to its whitelist. The same client can then reconnect later without a new invite.

##### Registry hashes

The client sends its block, biome, fluid, and tint registry hashes, and the server checks them against its own. A mismatch means the two sides disagree on what a given ID means, so the server disconnects the client with a checksum mismatch reason before either side can act on the disagreement. This protects both directions: it stops a mismatched client from receiving chunk data it would render as corrupted or nonsensical, and it stops that same client from placing a block ID the server does not recognize.

## Packet reference

Each packet starts with a 16-bit unsigned integer that names its ID. The packets are named and structured as follows:

### `0x0001` `StatusRequest`

|Type|Name|Description|
|----|----|----|
|`uint32`|`major`|Major game version|
|`uint32`|`minor`|Minor game version|
|`uint32`|`patch`|Patch game version|

### `0x0002` `StatusResponse`

|Type|Name|Description|
|----|----|----|
|`uint32`|`major`|Major game version|
|`uint32`|`minor`|Minor game version|
|`uint32`|`patch`|Patch game version|
|`uint32`|`tags`|Server tags|
|`string`|`motd`|One-line MOTD no longer than 32 characters|
|`uint16`|`players`|Current player count|
|`uint16`|`slots`|Maximum player slots|

##### Server flags

|Value|Description|
|----|----|
|`0x00000001`|Server is password-protected|
|`0x00000002`|Server has whitelist enabled|

### `0x0003` `AuthRequest`

|Type|Name|Description|
|----|----|----|
|`uint32`|`major`|Major game version|
|`uint32`|`minor`|Minor game version|
|`uint32`|`patch`|Patch game version|
|`uint8[32]`|`pkey`|Public Ed25519 key|
|`uint64`|`invite`|Invite-code|
|`uint64`|`biomes_hash`|Biome registry hash|
|`uint64`|`blocks_hash`|Block registry hash|
|`uint64`|`fluids_hash`|Fluid registry hash|
|`uint64`|`tints_hash`|Tint registry hash|
|`string`|`username`|Desired username|

### `0x0004` `AuthChallenge`

|Type|Name|Description|
|----|----|----|
|`uint8[64]`|`nonce`|Nonce for the client to sign|

### `0x0005` `AuthResponse`

|Type|Name|Description|
|----|----|----|
|`uint8[64]`|`signature`|Signed nonce + server password + UNIX minutes|

### `0x0006` `AuthAdmission`

|Type|Name|Description|
|----|----|----|
|`string`|`username`|Assigned username|

### `0x0007` `Disconnect`

|Type|Name|Description|
|----|----|----|
|`uint32`|`reason`|Disconnect reason|

##### Reason values

|Value|Description|When|
|----|----|----|
|`0x00000000`|Unspecified reason|Anything not specified below|
|`0x00000001`|Client disconnect|Client manually disconnects|
|`0x00000002`|Client shutdown|Client closes the game|
|`0x00000003`|Entity ID desync|EnTT registry goes nuts|
|`0x00000004`|Checksum mismatch|Client isn't synced up with server mods|
|`0x00000005`|Not whitelisted|Client's public key is not in the whitelist|
|`0x00000006`|Invalid signature|Client messed up nonce signing|
|`0x00000007`|Outdated client|Client is too old for the server|
|`0x00000008`|Outdated server|Client is too new for the server|
|`0x00000009`|Server is full|No more free slots available|
|`0x0000000A`|Server shutdown|Server is terminating|
