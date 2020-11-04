#!/usr/bin/env node

import { promisify, inspect } from 'util';
import * as path from 'path';

import { Trace } from './common.mjs';
import testcases from './testcases.mjs';

export const print = promisify(process.stdout.write.bind(process.stdout));

function exerciseAggregate(title, toMax, numberOfLines, results) {
  const lines = results.filter(([tc]) => tc.startsWith(title));
  if (lines.length !== numberOfLines) {
    throw new Error("Wrong number of testcases?");
  }

  const perLine = toMax / numberOfLines;
  let total = 0;
  const comments = [];
  for (const [, [score, max, comment]] of lines) {
    if (typeof score !== 'number' || typeof max !== 'number') {
      throw new Error(`Non-numeric score/max? ${inspect([score, max])}`);
    }
    total += score * perLine / max;
    if (comment) {
      comments.push(comment);
    }
  }
  total = Math.max(Math.min(total, toMax), 0);
  return { title, score: total, max: toMax, comment:
`Ex ${title}: ${round2dp(total)} / ${toMax}
${comments.map(l => `- ${l}`).join("\n")}
`.trim() };
}

export function round2dp(v) {
  return Math.round(v * 100) / 100;
}

export async function gradeSubmission(dirPath) {
  const results = await Promise.all(Object.entries(testcases).map(async ([tc, fn]) => {
    const trace = await Trace.fromFile(path.join(dirPath, `sm.${tc}.trace`));
    const result = fn(trace);

    const additionalComments = [];

    const exitSignal = trace.signal || trace.exitcodeSignal;
    if (exitSignal) {
      additionalComments.push(`Note: killed by ${exitSignal}`);
    }

    if (trace.readCommandFail) {
      additionalComments.push(`Note: runner failed to read line: "${trace.readCommandFail}" - possible bad FD manipulation`);
    }

    if (additionalComments.length > 0) {
      if (result[2]) {
        result[2] += "; ";
      } else {
        result[2] = `${tc}: `;
      }
      result[2] += additionalComments.join("; ");
    }
    return [tc, result];
  }));
  const aggregates = [
    exerciseAggregate("1a", 0.5, 5, results),
    exerciseAggregate("1b", 0.5, 2, results),
    exerciseAggregate("2a", 1.5, 4, results),
    exerciseAggregate("2b", 0.5, 2, results),
    exerciseAggregate("3", 1, 6, results),
    exerciseAggregate("4", 1, 4, results),
    exerciseAggregate("5", 1, 3, results)
  ];
  const total = round2dp(aggregates.reduce((x, y) => x + y.score, 0));
  return [total, "", aggregates.map(a => a.comment).join("\n").trim(), ...aggregates.map(a => round2dp(a.score))];
}
