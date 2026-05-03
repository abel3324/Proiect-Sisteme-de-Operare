# AI Usage Documentation

## Tool used

I used ChatGPT as an AI assistant for implementing the filter functionality, as required in Phase 1.

## Context

In my project, I have a struct for reports:

```c
typedef struct {
    int report_id;
    char inspector_name[50];
    double latitude;
    double longitude;
    char category[30];
    int severity;
    time_t timestamp;
    char description[100];
} Report;

The filter command receives conditions in the form:

field:operator:value

Example:
severity:>=:2

Supported fields:

severity
category
inspector
timestamp

Supported operators:

==, !=, <, <=, >, >=
First request to AI

I asked the AI to generate a function:

int parse_condition(const char *input, char *field, char *op, char *value);

The goal of this function is to split a string like:

severity:>=:2

into:

field = "severity"
op = ">="
value = "2"
What the AI generated

The AI used strchr() to find the ':' characters and then used strncpy() and strcpy() to extract the parts.

What I changed

I kept the main logic, but I added some basic checks:

return 0 if ':' is missing
return 0 if field or operator is empty
return 0 if value is empty

This avoids invalid inputs.

Second request to AI

I asked for another function:

int match_condition(Report *r, const char *field, const char *op, const char *value);

This function checks if a report matches a condition.

What the AI generated

The AI created logic to compare values based on field type:

integers (severity, timestamp)
strings (category, inspector)
What I changed

I split the logic into two helper functions:

compare_int()
compare_string()

This made the code easier to read.

For numbers:

I used atol() to convert value
compared using operators

For strings:

I used strcmp()
Filter logic (written by me)

The AI only helped with parsing and matching.

I implemented the actual filter logic myself:

open reports.dat
read each report using read()
for each condition:
parse using parse_condition()
check using match_condition()
print the report only if all conditions are true

Multiple conditions are combined using AND.

What I learned

I learned how to:

manually split strings in C using strchr
safely copy parts of strings
compare integers and strings based on operators
integrate AI-generated code into a real program

I also understood that AI code is not perfect and needs to be reviewed and adapted.