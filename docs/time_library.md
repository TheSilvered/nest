# Time and date library

## Importing

```text
|#| 'stdtime.nest' = dt
```

## Functions

### `[] @time`

Returns the time since the epoch in seconds.

### `[] @time_ns`

Returns the time since the epoch in nanoseconds.

### `[] @high_res_time`

Returns a timestamp with the highest resolution available in seconds. This value
has meaning only when compared with other values from the same function.

### `[] @high_res_time_ns`

Returns a timestamp with the highest resolution available in nanoseconds. This
value has meaning only when compared with other values from the same function.

### `[] @monotonic_time`

Returns a timestamp such that any later timestamps will be always equal or
greater. This can be for example the time since startup and similarly to
high_res_time has only meaning when compared with other timestamps from the same
function.

### `[] @monotonic_time_ns`

Returns a timestamp in nanoseconds such that any later timestamps will be always
equal or greater. This can be for example the time since startup and similarly
to high_res_time_ns has only meaning when compared with other timestamps from
the same function.

### `[] @year_day`

Returns the current day of the year, from `0` to `266`.

### `[] @week_day`

Returns the current day of the week, from `0`, Sunday, to `6`, Saturday.

### `[] @day`

Returns the current day of the month, from `1` to `31`.

### `[] @month`

Returns the current month, from `1`, January, to `12`, December.

### `[] @year`

Returns the current year.

### `[] @date`

Returns a map containing the current month day as `day`, week day as `week_day`,
year day as `year_day`, month as `month` and year as `year`.

### `[] @seconds`

Returns the current second, from `0` to `59`.

### `[] @minutes`

Returns the current minute, from `0` to `59`.

### `[] @hours`

Returns the current hour, from `0` to `23`.

### `[] @clock_time`

Returns a map containing the current second as `second`, the current minute as
`minute` and the current hour as `hour`.

### `[] @gmt_clock_time`

Returns a map containing the same information as `clock_time`, but the time is
taken with the Greenwich Main Time.

### `[] @clock_datetime`

Returns a map containing both the information from `clock_time` and `date`.

### `[] @gmt_clock_datetime`

Returns a map containing both the information from `gmt_clock_time` and `date`.  
The date is take with the Greenwich Main Time too, to there might be discrepancy
between `date` and `gmt_clock_datetime`.

### `[seconds: Int|Real] @sleep`

Pauses the execution for `seconds` seconds.

### `[milliseconds: Int|Real] @sleep_ms`

Pauses the execution for `milliseconds` milliseconds.

## Constants

### `MONTH_NAMES`

A map containing the complete month names.
`MONTH_NAMES.@@month --> July if '@@month' returns 7`.

### `SHORTH_MONTH_NAMES`

A map containing three-letter month names.

### `WEEKDAY_NAMES`

A map containing the complete week day names.

### `SHORT_WEEKDAY_NAMES`

A map containing three-letter week day name.
