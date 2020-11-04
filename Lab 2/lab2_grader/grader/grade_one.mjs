#!/usr/bin/env node

import { gradeSubmission, print } from './grader_common.mjs';

async function main() {
  const [score, , comment] = await gradeSubmission(".");
  await print(`Score: ${score} / 6

${comment}
`);
}

main();
