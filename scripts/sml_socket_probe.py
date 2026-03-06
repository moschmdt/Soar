#!/usr/bin/env python3

import argparse
import socket
import struct
import sys
import xml.etree.ElementTree as ET


class SMLSocketClient:
    def __init__(self, host: str, port: int, timeout: float = 5.0):
        self.host = host
        self.port = port
        self.timeout = timeout
        self.sock = None
        self.next_id = 1

    def connect(self) -> None:
        self.sock = socket.create_connection(
            (self.host, self.port), timeout=self.timeout
        )
        self.sock.settimeout(self.timeout)

    def close(self) -> None:
        if self.sock is not None:
            self.sock.close()
            self.sock = None

    def _recv_exact(self, size: int) -> bytes:
        data = bytearray()
        while len(data) < size:
            chunk = self.sock.recv(size - len(data))
            if not chunk:
                raise ConnectionError("Connection closed while reading framed message")
            data.extend(chunk)
        return bytes(data)

    def send_xml(self, xml_payload: str) -> None:
        payload = xml_payload.encode("utf-8")
        header = struct.pack("!I", len(payload))
        self.sock.sendall(header + payload)

    def recv_xml(self) -> str:
        header = self._recv_exact(4)
        (length,) = struct.unpack("!I", header)
        if length == 0:
            return ""
        payload = self._recv_exact(length)
        return payload.decode("utf-8", errors="replace")

    def _build_call(
        self, command_name: str, args: list[tuple[str, str]], output: str | None = None
    ) -> tuple[int, str]:
        request_id = self.next_id
        self.next_id += 1

        root = ET.Element(
            "sml",
            {
                "doctype": "call",
                "id": str(request_id),
                "smlversion": "0.0.0",
            },
        )

        command_attrs = {"name": command_name}
        if output:
            command_attrs["output"] = output
        command = ET.SubElement(root, "command", command_attrs)

        for param, value in args:
            arg = ET.SubElement(command, "arg", {"param": param})
            arg.text = value

        xml_text = ET.tostring(root, encoding="unicode")
        return request_id, xml_text

    def call(
        self,
        command_name: str,
        args: list[tuple[str, str]] | None = None,
        output: str | None = None,
    ) -> ET.Element:
        if args is None:
            args = []

        request_id, xml_msg = self._build_call(command_name, args, output)
        self.send_xml(xml_msg)

        while True:
            response_xml = self.recv_xml()
            if not response_xml:
                continue

            root = ET.fromstring(response_xml)
            doctype = root.attrib.get("doctype")
            ack = root.attrib.get("ack")

            if doctype == "response" and ack == str(request_id):
                return root

    @staticmethod
    def summarize_response(root: ET.Element) -> tuple[bool, str]:
        error = root.find("error")
        if error is not None:
            return False, "".join(error.itertext()).strip()

        result = root.find("result")
        if result is None:
            return True, "<no result tag>"

        output_type = result.attrib.get("output", "raw")
        if output_type == "structured":
            names = [
                "".join(node.itertext()).strip() for node in result.findall("name")
            ]
            names = [name for name in names if name]
            if names:
                return True, f"agents={names}"
            return True, ET.tostring(result, encoding="unicode")

        text = "".join(result.itertext()).strip()
        return True, text


def run_probe(
    host: str, port: int, agent: str | None, cmdline: str | None, timeout: float
) -> int:
    client = SMLSocketClient(host, port, timeout)
    try:
        client.connect()
        print(f"Connected to {host}:{port}")

        version_response = client.call("version")
        ok, summary = client.summarize_response(version_response)
        print(f"version: {'ok' if ok else 'error'} -> {summary}")
        if not ok:
            return 2

        list_response = client.call("get_agent_list")
        ok, summary = client.summarize_response(list_response)
        print(f"get_agent_list: {'ok' if ok else 'error'} -> {summary}")
        if not ok:
            return 3

        if cmdline:
            if not agent:
                print("cmdline requested but no --agent provided", file=sys.stderr)
                return 4
            args = [("agent", agent), ("line", cmdline)]
            cmd_response = client.call("cmdline", args=args, output="raw")
            ok, summary = client.summarize_response(cmd_response)
            print(f"cmdline: {'ok' if ok else 'error'} -> {summary}")
            if not ok:
                return 5

        return 0
    except TimeoutError:
        print(
            (
                "Probe failed: timed out. If using SoarCLI, launch with "
                "'./soar -l -p <port>' so remote SML requests are serviced; "
                "note '-n' only disables color and does not set the port."
            ),
            file=sys.stderr,
        )
        return 1
    except socket.timeout:
        print(
            (
                "Probe failed: timed out. If using SoarCLI, launch with "
                "'./soar -l -p <port>' so remote SML requests are serviced; "
                "note '-n' only disables color and does not set the port."
            ),
            file=sys.stderr,
        )
        return 1
    except Exception as exc:
        print(f"Probe failed: {exc}", file=sys.stderr)
        return 1
    finally:
        client.close()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Probe Soar SML remote XML protocol over raw sockets"
    )
    parser.add_argument(
        "--host", default="127.0.0.1", help="Kernel host (default: 127.0.0.1)"
    )
    parser.add_argument(
        "--port", type=int, default=12121, help="Kernel listener port (default: 12121)"
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="Socket timeout in seconds (default: 5.0)",
    )
    parser.add_argument("--agent", default=None, help="Agent name for --cmdline")
    parser.add_argument(
        "--cmdline",
        default=None,
        help="Optional Soar cmdline to execute (requires --agent)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    return run_probe(args.host, args.port, args.agent, args.cmdline, args.timeout)


if __name__ == "__main__":
    raise SystemExit(main())
