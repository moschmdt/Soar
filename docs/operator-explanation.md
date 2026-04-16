# Operator Explanation

The **OperatorExplanationManager** records why specific operators were selected
(or rejected) each decision cycle. Data is held in an in-memory SQLite database
and can be queried at any point during or after a run.

## 1. Enable Saving Data for an Operator

Call `explain track-operator` with the operator name **before** running:

```shell
explain track-operator <name>
```

### Track all operators (exclusion-first workflow)

Use the command-line argument `--all` (short form `-a`) to track every
operator by default:

```shell
explain track-operator --all
explain track-operator -a
```

In this mode, `untrack-operator` works as an exclusion list:

```shell
explain untrack-operator <name>
```

That excludes `<name>` from tracking while all other operators remain tracked.
To re-include an excluded operator, call:

```shell
explain track-operator <name>
```

To disable all-mode and return to normal explicit tracking:

```shell
explain untrack-operator all
```

The database is initialised lazily on the first `track-operator` call, so no
setup is needed other than this command.

**Example**

```shell
explain track-operator move-north
explain track-operator stay-put
```

### View the tracking list

Call `explain track-operator` with no argument to see which operators are
currently tracked:

```shell
explain track-operator
```

### Stop tracking an operator

```shell
explain untrack-operator <name>
```

In normal mode this removes the operator from the watch list. In all-mode it
adds the operator to the exclusion list. Previously recorded decisions are not
deleted and remain queryable.

> **Timing matters.** Tracking takes effect for the _next_ decision cycle, so
> the command must be issued before `run`. Data is not captured retroactively.

---

## 2. Generate the Explanation (CLI)

### Text output

```shell
explain operator <name>
```

Prints a human-readable summary of every recorded decision cycle in which
`<name>` was either the winner or a losing candidate. For each decision you see:

- The decision cycle number, goal level, and state ID
- Whether the operator won or an impasse occurred
- Every candidate that was proposed: its WME ID, proposing rule, and the LHS
  conditions that matched
- The reason each losing candidate was eliminated (reject, prohibit, dominated,
  not-best, …)
- The selection preferences (OSK) that were active at the time

**Example session**

```shell
explain track-operator move-north
source my-agent.soar
run 10
explain operator move-north
```

**Sample output**

```shell
=== Decision at cycle 3  [level 2, state S1] ===
  WINNER: move-north (O3)  [propose*move-north]

  [WINNER]  move-north (O3)  proposed by: propose*move-north
            LHS matched:
              (S1 ^type state)
              (S1 ^location room-a)

  [LOSER]   stay-put (O4)  proposed by: propose*stay-put
            eliminated: reject [apply*reject-stay-put]
            LHS matched:
              (S1 ^type state)

  Selection preferences:
    best(move-north)  [prefer-north]
```

If the operator was tracked but no decisions were recorded (e.g. the agent
never proposed it), the output is:

```shell
No decisions recorded for operator 'move-north'.
  Make sure 'explain track-operator move-north' was called before running.
```

If `track-operator` was never called at all (the database was never
initialised), the output is:

```shell
No explanation data recorded yet.  Use 'explain track-operator move-north' first.
```

---

### JSON output

Append `--json` (short form: `-j`) to get machine-readable output:

```shell
explain operator <name> --json
explain operator <name> -j
```

The JSON object always has a `decisions` array. When data is present each
element describes one decision cycle:

```json
{
  "operator": "move-north",
  "decisions": [
    {
      "cycle": 3,
      "goal_level": 2,
      "state_id": "S1",
      "winner": "move-north",
      "winner_op_id": "O3",
      "winner_rule": "propose*move-north",
      "impasse_type": "none",
      "candidates": [
        {
          "op_id": "O3",
          "op_name": "move-north",
          "proposal_rule": "propose*move-north",
          "is_winner": true,
          "elimination": null,
          "conditions": [
            { "negated": false, "id": "S1", "attr": "type", "value": "state" },
            {
              "negated": false,
              "id": "S1",
              "attr": "location",
              "value": "room-a"
            }
          ]
        },
        {
          "op_id": "O4",
          "op_name": "stay-put",
          "proposal_rule": "propose*stay-put",
          "is_winner": false,
          "elimination": "reject [apply*reject-stay-put]",
          "conditions": [
            { "negated": false, "id": "S1", "attr": "type", "value": "state" }
          ]
        }
      ],
      "preferences": [
        {
          "type": "best",
          "value_id": "O3",
          "value_name": "move-north",
          "referent_id": null,
          "referent_name": null,
          "producing_rule": "prefer-north"
        }
      ]
    }
  ]
}
```

When no decisions exist but tracking was active, `decisions` is an empty array:

```json
{ "operator": "move-north", "decisions": [] }
```

When the database was never initialised (no operator was ever tracked), the
response includes an `error` field:

```json
{ "decisions": [], "error": "No explanation data recorded yet." }
```

---

## Notes

- All data is **in-memory only** and is lost when the agent is destroyed or
  `init-soar` is called. There is no persistence across sessions.
- Tracking survives `init-soar` — the watch list is not cleared on
  re-initialisation.
- Multiple operators can be tracked simultaneously.
- Decisions are recorded for a cycle if **any** tracked operator appears as
  either the winner or a losing candidate in that cycle.
