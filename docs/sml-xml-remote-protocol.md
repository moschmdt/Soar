# Soar SML Remote XML Protocol (for external adapters)

This note documents the practical wire protocol used by Soar SML remote connections, based on CMake source inspection.

## Scope

- Goal: communicate with a running Soar kernel from an external client (e.g., VS Code debug adapter in Node/TypeScript) **without** linking or loading Soar client libraries.
- Transport: TCP socket to kernel listener port (default `12121`).
- Message format: framed XML (not line-delimited XML).

## Transport framing

Each message on the socket is:

1. `uint32` message length in **network byte order** (big-endian)
2. exactly `length` bytes of XML payload (UTF-8 bytes in practice)

References:

- `Core/ConnectionSML/src/sock_DataSender.cpp` (`SendString`, `ReceiveString`)
- `Core/ConnectionSML/src/sml_RemoteConnection.cpp` (`SendMsg`, `ReceiveMessages`)

## Top-level XML envelope

Root tag is `<sml>` with attributes:

- `doctype` in `{call,response,notify}`
- `id` (string/integer message id)
- `ack` on responses (the request id being acknowledged)
- `smlversion`

Identifiers from `Core/ConnectionSML/src/sml_Names.cpp`.

Typical request shape:

```xml
<sml smlversion="x.y.z" doctype="call" id="42">
  <command name="version"/>
</sml>
```

Typical response shape:

```xml
<sml smlversion="x.y.z" doctype="response" id="43" ack="42">
  <result output="raw">9.6.3</result>
</sml>
```

## Command structure

Commands are encoded under `<command>`:

- Required attribute: `name`
- Optional attribute: `output="raw"|"structured"`
- Optional `<arg>` children

Argument encoding:

```xml
<arg param="line" type="string">watch 1</arg>
```

(`type` is optional for most handlers.)

Construction path:

- `Connection::CreateSMLCommand`
- `Connection::AddParameterToSMLCommand`

## Response patterns

### Raw scalar responses

Most command handlers return:

```xml
<result output="raw">...</result>
```

via `AddSimpleResultToSMLResponse`.

### Structured responses

Some handlers return structured XML with:

```xml
<result output="structured">...</result>
```

Example: `get_agent_list` returns `<name>` children in `<result>`.

### Error responses

Errors include both:

- `<result output="raw">error text</result>`
- `<error code="...">error text</error>` (code may be omitted)

Clients should treat presence of `<error>` as failure.

## Useful starter commands

- `version` (safe connectivity check)
- `get_agent_list` (structured response)
- `cmdline` with args:
  - `param="agent"` = agent name
  - `param="line"` = command line text

`cmdline` is handled by `KernelSML::HandleCommandLine`.

## Correlation and concurrency notes

- Responses are matched by `ack == request.id`.
- Server-generated `id` in responses is independent from client request id.
- Client should keep monotonically increasing request ids and match by `ack`.
- Kernel may send asynchronous callbacks/events when registered; robust clients should be prepared to receive non-matching messages while waiting for a specific `ack`.

## Implications for a VS Code debug adapter

You can implement a standalone transport layer in TypeScript/Node:

1. open socket
2. implement 4-byte big-endian length framing
3. emit/parse SML XML envelopes
4. correlate replies by `ack`

No Soar native library is required in the extension process for remote mode.

## Limitations

- This protocol is internal and not currently versioned as a separate public spec.
- Command semantics are defined by kernel handlers; changes in future Soar versions may require adapter updates.

## Machine-readable companion files

- JSON schema for catalog format: `docs/sml-xml-command-catalog.schema.json`
- Generated command catalog instance: `docs/sml-xml-command-catalog.json`

The command catalog is derived from `KernelSML::BuildCommandMap()` plus handler argument usage in `sml_KernelSMLHandlers.cpp`.
