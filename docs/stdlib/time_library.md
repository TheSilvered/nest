# Date and time library

## Importing

```nest
|#| 'stdtime.nest' = dt
```

## Functions

### `@clock_datetime`

**Synopsis:**

```nest
[timestamp: Int?] @clock_datetime -> Map
```

**Returns:**

A map containing both the information from [`clock_time`](#clock_time) and
[`date`](#date).

If `timestamp` is given it is used to calculate the values otherwise the
current time is used.

---

### `@clock_time`

**Synopsis:**

```nest
[timestamp: Int?] @clock_time -> Map
```

**Returns:**

A map containing the current second as `second`, the current minute as `minute`
and the current hour as `hour`.

If `timestamp` is given it is used to calculate the values otherwise the
current time is used.

---

### `@date`

**Synopsis:**

```nest
[timestamp: Int?] @date -> Map
```

**Returns:**

A map containing the current day of the month as `day`, day of the week as
`week_day`, day of the year as `year_day`, month as `month` and year as `year`.

If `timestamp` is given it is used to calculate the values otherwise the
current time is used.

---

### `@day`

**Synopsis:**

```nest
[timestamp: Int?] @day -> Int
```

**Returns:**

The current day of the month, from `1` to `31`.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@gmt_clock_datetime`

**Synopsis:**

```nest
[timestamp: Int?] @gmt_clock_datetime -> Map
```

**Returns:**

A map containing both the information from [`gmt_clock_time`](#gmt_clock_time)
and [`gmt_date`](#gmt_date).

If `timestamp` is given it is used to calculate the values otherwise the
current time is used.

---

### `@gmt_clock_time`

**Synopsis:**

```nest
[timestamp: Int?] @gmt_clock_time -> Map
```

**Returns:**

A map containing the same information as [`clock_time`](#clock_time), but
taken with the Greenwich Main Time.

If `timestamp` is given it is used to calculate the values otherwise the
current time is used.

---

### `@gmt_date`

**Synopsis:**

```nest
[timestamp: Int?] @gmt_date -> Map
```

**Returns:**

A map containing the same information as [`date`](#date), but taken with the
Greenwich Main Time.

If `timestamp` is given it is used to calculate the values otherwise the
current time is used.

---

### `@high_res_time`

**Synopsis:**

```nest
[] @high_res_time -> Real
```

**Returns:**

A timestamp with the highest resolution available in seconds. This value has
meaning only when compared with other values from the same function.

---

### `@high_res_time_ns`

**Synopsis:**

```nest
[] @high_res_time_ns -> Int
```

**Returns:**

A timestamp with the highest resolution available in nanoseconds. This value
has meaning only when compared with other values from the same function.

---

### `@hour`

**Synopsis:**

```nest
[timestamp: Int?] @hour -> Int
```

**Returns:**

The current hour, from `0` to `23` in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@minute`

**Synopsis:**

```nest
[timestamp: Int?] @minute -> Int
```

**Returns:**

The current minute, from `0` to `59` in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@monotonic_time`

**Synopsis:**

```nest
[] @monotonic_time -> Real
```

**Returns:**

A timestamp in seconds such that any later timestamps will be always equal to
or greater greater than previously take ones. This can be for example the time
since startup. The return value has only meaning when compared with other
timestamps from the same function.

---

### `@monotonic_time_ns`

**Synopsis:**

```nest
[] @monotonic_time_ns -> Int
```

**Returns:**

A timestamp in nanoseconds such that any later timestamps will be always equal
to or greater greater than previously take ones. This can be for example the
time since startup. The return value has only meaning when compared with other
timestamps from the same function.

---

### `@month`

**Synopsis:**

```nest
[timestamp: Int?] @month -> Int
```

**Returns:**

The current month, from `1` (January) to `12` (December) in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@second`

**Synopsis:**

```nest
[timestamp: Int?] @second -> Int
```

**Returns:**

The current second, from `0` to `61` in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@sleep`

**Synopsis:**

```nest
[seconds: Byte|Int|Real] @sleep -> null
```

**Description:**

Pauses the execution for `seconds` seconds.

---

### `@sleep_ms`

**Synopsis:**

```nest
[milliseconds: Byte|Int] @sleep_ms -> null
```

**Description:**

Pauses the execution for `milliseconds` milliseconds.

---

### `@time`

**Synopsis:**

```nest
[] @time -> Real
```

**Returns:**

The time since the epoch in seconds.

---

### `@time_ns`

**Synopsis:**

```nest
[] @time_ns -> Int
```

**Returns:**

The time since the epoch in nanoseconds.

---

### `@week_day`

**Synopsis:**

```nest
[timestamp: Int?] @week_day -> Int
```

**Returns:**

The current day of the week, from `0` (Sunday) to `6` (Saturday) in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@year`

**Synopsis:**

```nest
[timestamp: Int?] @year -> Int
```

**Returns:**

The current year in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

### `@year_day`

**Synopsis:**

```nest
[timestamp: Int?] @year_day -> Int
```

**Returns:**

The current day of the year, from `0` to `365` in local time.

If `timestamp` is given it is used to calculate the value otherwise the
current time is used.

---

## Constants

### `MONTH_NAMES`

A map containing the complete month names.
`MONTH_NAMES.@@month --> July if '@@month' returns 7`.

---

### `SHORT_MONTH_NAMES`

A map containing three-letter month names.

---

### `WEEKDAY_NAMES`

A map containing the complete week day names.

---

### `SHORT_WEEKDAY_NAMES`

A map containing three-letter week day name.
