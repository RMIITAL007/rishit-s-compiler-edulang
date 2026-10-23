import { SampleProgram } from './types';

export const SAMPLES: SampleProgram[] = [
  {
    id: 'arithmetic_control',
    title: 'Arithmetic & Control Flow (arithmetic_control.edu)',
    category: 'Standard Programs',
    description: 'Constant arithmetic (1 + 2), if/else branching, multiplication, and print statement. Demonstrates 5.9% constant folding.',
    code: `int a = 1 + 2;
if (a < 5) {
	print(a * 2);
} else {
	print(0);
}
`,
  },
  {
    id: 'hello',
    title: 'Variable Addition (hello.edu)',
    category: 'Standard Programs',
    description: 'Basic variable declarations, symbol table binding, and addition.',
    code: `int a = 10;
int b = 20;
print(a + b);
`,
  },
  {
    id: 'nested_loops',
    title: 'Nested While Loops (nested_loops.edu)',
    category: 'Standard Programs',
    description: 'Multi-level nested while loops with inner and outer loop counters.',
    code: `int outer = 0;
while (outer < 2) {
    int inner = 0;
    while (inner < 2) {
        print(outer * 10 + inner);
        inner = inner + 1;
    }
    outer = outer + 1;
}
`,
  },
  {
    id: 'nested_boolean',
    title: 'Boolean Logic & Short-Circuit (nested_boolean.edu)',
    category: 'Standard Programs',
    description: 'Complex boolean expressions with &&, ||, ! operators and conditional branching.',
    code: `int a = 10;
if ((a > 5 && true) || false) {
    if (!(a == 0)) {
        print(a);
    } else {
        print(0);
    }
}
`,
  },
  {
    id: 'else_branch',
    title: 'Else Branching (else_branch.edu)',
    category: 'Standard Programs',
    description: 'Comparison operator < with an alternative else block execution.',
    code: `int value = 10;
if (value < 5) {
    print(value);
} else {
    print(99);
}
`,
  },
  {
    id: 'syntax_error',
    title: 'Missing Expression (syntax_error.edu)',
    category: 'Diagnostic Errors',
    description: 'Syntax error recovering with line, column, caret, and educational hint.',
    code: `int value = ;
`,
  },
  {
    id: 'semantic_error',
    title: 'Undeclared Variable (semantic_error.edu)',
    category: 'Diagnostic Errors',
    description: 'Semantic error detecting reference to an undeclared identifier.',
    code: `print(missing_value);
`,
  },
  {
    id: 'type_mismatch',
    title: 'Type Mismatch (type_mismatch.edu)',
    category: 'Diagnostic Errors',
    description: 'Type checking preventing addition between integer and boolean.',
    code: `int a = 10;
int b = true;
print(a + b);
`,
  },
  {
    id: 'runtime_error',
    title: 'Division by Zero (runtime_error.edu)',
    category: 'Diagnostic Errors',
    description: 'Runtime VM safety guard catching division by zero.',
    code: `print(10 / 0);
`,
  },
  {
    id: 'timeout_loop',
    title: 'Infinite Loop (Subprocess Timeout Guard)',
    category: 'Stress & Robustness',
    description: 'An unbounded while loop triggering the strict 2000ms sandbox execution timeout.',
    code: `int x = 1;
while (x == 1) {
    x = 1;
}
`,
  },
];

export const DEFAULT_SAMPLE = SAMPLES[0];
