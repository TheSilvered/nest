# Time and date library

## Importing

```nest
|#| 'stdtime.nest' = dt
```

## Functions

### `@clock_datetime`

**Synopsis**:

`[] @clock_datetime -> Map`

**Return value**:

Returns a map containing both the information from `clock_time` and `date`.

---

### `@clock_time`

**Synopsis**:

`[] @clock_time -> Map`

**Return value**:

Returns a map containing the current second as `second`, the current minute as
`minute` and the current hour as `hour`.

---

### `@date`

**Synopsis**:

`[] @date -> Map`

**Return value**:

Returns a map containing the current month day as `day`, week day as `week_day`,
year day as `year_day`, month as `month` and year as `year`.

---

### `@day`

**Synopsis**:

`[] @day -> Int`

**Return value**:

Returns the current day of the month, from `1` to `31`.

---

### `@gmt_clock_datetime`

**Synopsis**:

`[] @gmt_clock_datetime -> Map`

**Return value**:

Returns a map containing both the information from `gmt_clock_time` and `date`.  
The date is take with the Greenwich Main Time too, to there might be discrepancy
between `date` and `gmt_clock_datetime`.

---

### `@gmt_clock_time`

**Synopsis**:

`[] @gmt_clock_time -> Map`

**Return value**:

Returns a map containing the same information as `clock_time`, but the time is
taken with the Greenwich Main Time.

---

### `@high_res_time`

**Synopsis**:

`[] @high_res_time -> Real`

**Return value**:

Returns a timestamp with the highest resolution available in seconds. This value
has meaning only when compared with other values from the same function.

---

### `@high_res_time_ns`

**Synopsis**:

`[] @high_res_time_ns -> Int`

**Return value**:

Returns a timestamp with the highest resolution available in nanoseconds. This
value has meaning only when compared with other values from the same function.

---

### `@hours`

**Synopsis**:

`[] @hours -> Int`

**Return value**:

Returns the current hour, from `0` to `23`.

---

### `@minutes`

**Synopsis**:

`[] @minutes -> Int`

**Return value**:

Returns the current minute, from `0` to `59`.

---

### `@monotonic_time`

**Synopsis**:

`[] @monotonic_time -> Real`

**Return value**:

Returns a timestamp such that any later timestamps will be always equal or
greater. This can be for example the time since startup and similarly to
high_res_time has only meaning when compared with other timestamps from the same
function.

---

### `@monotonic_time_ns`

**Synopsis**:

`[] @monotonic_time_ns -> Int`

**Return value**:

Returns a timestamp in nanoseconds such that any later timestamps will be always
equal or greater. This can be for example the time since startup and similarly
to high_res_time_ns has only meaning when compared with other timestamps from
the same function.

---

### `@month`

**Synopsis**:

`[] @month -> Int`

**Return value**:

Returns the current month, from `1`, January, to `12`, December.

---

### `@seconds`

**Synopsis**:

`[] @seconds -> Int`

**Return value**:

Returns the current second, from `0` to `59`.

---

### `@sleep`

**Synopsis**:

`[seconds: Byte|Int|Real] @sleep -> null`

**Description**:

Pauses the execution for `seconds` seconds.

---

### `@sleep_ms`

**Synopsis**:

`[milliseconds: Byte|Int] @sleep_ms -> null`

**Description**:

Pauses the execution for `milliseconds` milliseconds.

---

### `@time`

**Synopsis**:

`[] @time -> Real`

**Return value**:

Returns the time since the epoch in seconds.

---

### `@time_ns`

**Synopsis**:

`[] @time_ns -> Int`

**Return value**:

Returns the time since the epoch in nanoseconds.

---

### `@week_day`

**Synopsis**:

`[] @week_day -> Int`

**Return value**:

Returns the current day of the week, from `0`, Sunday, to `6`, Saturday.

---

### `@year`

**Synopsis**:

`[] @year -> Int`

**Return value**:

Returns the current year.

---

### `@year_day`

**Synopsis**:

`[] @year_day -> Int`

**Return value**:

Returns the current day of the year, from `0` to `266`.

---

## Constants

### `MONTH_NAMES`

A map containing the complete month names.
`MONTH_NAMES.@@month --> July if '@@month' returns 7`.

---

### `SHORTH_MONTH_NAMES`

A map containing three-letter month names.

---

### `WEEKDAY_NAMES`

A map containing the complete week day names.

---

### `SHORT_WEEKDAY_NAMES`

A map containing three-letter week day name.
