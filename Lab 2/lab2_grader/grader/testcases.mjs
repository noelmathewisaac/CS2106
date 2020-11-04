import { createHash } from 'crypto';
import { arrayEqual } from './common.mjs';

Array.prototype.findLast = function (pred) {
  const t = this;
  for (let i = t.length - 1; i >= 0; --i) {
    if (pred(t[i])) {
      return t[i];
    }
  }
  return undefined;
};

const testcases = {};
export default testcases;

const F = "FACTOR";

const paths = process.env.TEST_BIN_ALREADY_SET ? ({
  sf: "./___sf",
  sfa: "./___sfa",
  sfb: "./___sfb",
  sfc: "./___sfc",
  echo: "/bin/echo",
  sleep: "/bin/sleep"
}) : ({
  sf: "/tmp/sf",
  sfa: "/tmp/sfa",
  sfb: "/tmp/sfb",
  sfc: "/tmp/sfc",
  echo: "/bin/echo",
  sleep: "/bin/sleep"
});

function sha256sumOutput(input) {
  const h = createHash('sha256');
  h.update(input, 'utf8');
  return `${h.digest('hex')}  -`;
}

function commandArgv(t, i) {
  return t.commands[i].split(" ").slice(1);
}

function joinComments(title, comments) {
  return `${title}: ${comments.join("; ")}`;
}

function equalAggregate(title, toMax, numResults, ...results) {
  const comments = [];
  const perResult = toMax / numResults;
  let seenResults = 0;
  let factor = 1;
  let totalScore = 0;
  for (const [score, max, comment] of results.filter(x => !!x)) {
    if (score === F) {
      factor *= max;
      comments.push(`*${max} penalty: ${comment}`);
      continue;
    }

    totalScore += (score / max) * perResult;
    if (comment) {
      comments.push(comment);
    }
    ++seenResults;
  }
  if (seenResults !== numResults) {
    throw new Error(`Wrong number of results`);
  }
  totalScore *= factor;
  totalScore = Math.min(Math.max(0, totalScore), toMax);
  if (totalScore < toMax || comments.length > 0) {
    return [totalScore, toMax, joinComments(title, comments)];
  }
  return [totalScore, toMax];
}

// EXERCISE 1A

function ex1CheckExec(t, ...argv) {
  const path = argv[0];
  const exec = t.findExec(path);
  if (!exec) { return [0, 1, `Missing exec ${argv[0]}`]; }
  exec.seen = true;
  if (exec.isMainProcess) { return [-Infinity, 0, "Parent process exec-ed"]; }
  if (arrayEqual(exec.argv, argv)) { return [1, 1]; }
  const argvNoArgv0 = argv.slice(1);
  if (arrayEqual(exec.argv.slice(1), argvNoArgv0)) { return [0.75, 1, `Wrong argv[0] in exec ${argv[0]}`]; }
  if (arrayEqual(exec.argv.slice(1, argv.length - 1), argvNoArgv0)) { return [0.5, 1, `Extraneous arguments in exec ${argv[0]}`]; }
  if (arrayEqual(exec.argv.slice(0, argv.length - 1), argvNoArgv0)) { return [0.25, 1, `Missing argv[0] in exec ${argv[0]}`]; }
  return [0, 1, `Wrong exec ${exec.argv.join(" ")}`];
}

function ex1CheckExecs(t) {
  return t.commands.filter(c => c.startsWith("start")).map(c => ex1CheckExec(t, ...c.split(" ").slice(1)));
}

function ex1CheckExtraExecs(t) {
  if (t.getUnseenExecs().length > 0) {
    return [F, 0.5, "Extraneous execs seen"];
  }
  return null;
}

Object.assign(testcases, {
  '1a1': t => equalAggregate('1a1', 1, 1,
    ...ex1CheckExecs(t),
    ex1CheckExtraExecs(t)
  ),
  '1a2': t => equalAggregate('1a2', 1, 1,
    ...ex1CheckExecs(t),
    ex1CheckExtraExecs(t)
  ),
  '1a3': t => equalAggregate('1a3', 1, 1,
    ...ex1CheckExecs(t),
    ex1CheckExtraExecs(t)
  ),
  '1a4': t => equalAggregate('1a4', 1, 3,
    ...ex1CheckExecs(t),
    ex1CheckExtraExecs(t)
  ),
  '1a5': t => equalAggregate('1a5', 1, 1,
    t.hasRawLine(`print-${commandArgv(t, 0)[1]}`)
      ? [1, 1]
      : [0, 1, "Expected output not seen"]
  ),
});

// EXERCISE 1B, 2B

const statusString = {
  true: "Running",
  false: "Exited"
};

function ex1bCheckStatuses(t, c, i, p, expected) {
  const status = t.findStatus(c, i);
  if (!status) { return [0, 1, `Missing status for ${p} at command index ${c}`]; }
  const exec = t.findExec(p);
  if (!exec) { return [0, 1, `Missing exec ${p}`]; }
  if (status.running !== expected) { return [0, 1, `Expected ${statusString[expected]}, got ${statusString[status.running]} for ${p} at command index ${c}`]; }
  if (status.pid !== exec.pid || status.path !== p) { return [0.5, 1, `Wrong PID or path for ${p} at command index ${c}`]; }
  return [1, 1];
}

Object.assign(testcases, {
  '1b1': t => equalAggregate('1b1', 1, 2,
    ex1bCheckStatuses(t, 1, 0, paths.sf, true),
    ex1bCheckStatuses(t, 2, 0, paths.sf, false)
  ),
  '1b2': t => equalAggregate('1b2', 1, 4,
    ex1bCheckStatuses(t, 2, 1, paths.sf, true),
    ex1bCheckStatuses(t, 3, 1, paths.sf, false),
    ex1bCheckStatuses(t, 2, 0, paths.echo, false),
    ex1bCheckStatuses(t, 3, 0, paths.echo, false)
  ),
  '2b1': t => equalAggregate('2b1', 1, 4,
    ex1bCheckStatuses(t, 1, 0, paths.sfc, true),
    ex1bCheckStatuses(t, 2, 0, paths.sfc, true),
    ex1bCheckStatuses(t, 3, 0, paths.sfc, true),
    ex1bCheckStatuses(t, 4, 0, paths.sfc, false),
  ),
  '2b2': t => equalAggregate('2b2', 1, 4,
    ex1bCheckStatuses(t, 1, 0, paths.sfc, true),
    ex1bCheckStatuses(t, 2, 0, paths.sfc, false),
    ex1bCheckStatuses(t, 3, 0, paths.sfc, false),
    ex1bCheckStatuses(t, 4, 0, paths.sfc, false),
  ),
});

// EXERCISE 2A

function ex2CheckExec(t, ...argv) {
  const path = argv[0];
  const execs = t.filterExecs(path);
  if (execs.length == 0) { return `Missing exec ${argv[0]}`; }
  for (const exec of execs) {
    if (exec.isMainProcess) {
      exec.seen = true;
      return "Parent process exec-ed";
    }
    const argvNoArgv0 = argv.slice(1);
    if (arrayEqual(exec.argv, argv)
      || arrayEqual(exec.argv.slice(1), argvNoArgv0)
      || arrayEqual(exec.argv.slice(1, argv.length - 1), argvNoArgv0)
      || arrayEqual(exec.argv.slice(0, argv.length - 1), argvNoArgv0)) {
      exec.seen = true;
      return;
    }
  }
  return `Missing exec ${argv[0]}`;
}

function ex2CheckExecs(t) {
  const execs = t.commands.filter(c => c.startsWith("start")).flatMap(c => c.substring(6).split(" | ").map(exec => exec.split(" ")));
  const fails = [];
  for (const exec of execs) {
    const failReason = ex2CheckExec(t, ...exec);
    if (failReason) {
      fails.push(failReason);
    }
  }
  if (t.getUnseenExecs().length > 0) {
    fails.push("Extraneous execs seen");
  }
  return fails;
}

function makeEx2(title, outputFn) {
  return t => {
    const fails = ex2CheckExecs(t);
    const execFail = fails.length > 0;
    let outputFail = false;
    const output = outputFn(t);
    if (!t.hasRawLine(output)) {
      fails.push("Expected output not found");
      outputFail = true;
    }
    const failStr = joinComments(title, fails);
    if (outputFail && execFail) {
      return [0, 1, failStr];
    } else if (outputFail || execFail) {
      return [0.5, 1, failStr];
    } else {
      return [1, 1];
    }
  };
}

Object.assign(testcases, {
  '2a1': makeEx2('2a1', t => sha256sumOutput(t.commands[0].split(" ")[2] + '\n')),
  '2a2': makeEx2('2a2', t => sha256sumOutput(t.commands[0].split(" ")[17] + '\n')),
  '2a3': makeEx2('2a3', t => "print-hello"),
  '2a4': makeEx2('2a4', t => `periodic-write-${t.commands[0].split(" ")[2]}`),
});

// EXERCISE 3A

function makeEx3a(title) {
  return t => {
    const fails = ex2CheckExecs(t);
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    const firstStopCommandIndex = t.commands.findIndex(x => x.startsWith("stop"));
    if (firstStopCommandIndex === -1) {
      throw new Error("No stop command?");
    }
    let stopEndIndex = Infinity;
    let firstStopSeen = false;
    const children = {};
    for (const line of t.trace) {
      switch (line.type) {
      case 'clone':
      case 'execve': {
        children[line.pid] = line;
        line.killed = false;
        line.exited = false;
        break;
      }

      case 'kill': {
        if (firstStopSeen) {
          fails.push("Kill seen after first stop command");
        }
        const child = children[line.pid];
        if (!child) {
          fails.push(`Wrong PID ${line.pid} killed`);
          break;
        }
        if (child.exited) {
          fails.push(`Killing exited child ${child.path} PID ${line.pid}`);
        }
        if (['SIGTERM', 'SIGKILL'].includes(line.signal)) {
          child.killed = true;
        }
        break;
      }

      case 'sigchld': {
        const child = children[line.pid];
        if (child) {
          child.exited = true;
        }
        break;
      }

      case 'commandEnd': {
        if (line.index === firstStopCommandIndex) {
          firstStopSeen = true;
          stopEndIndex = line.lineIndex;
        }
        break;
      }

      case 'pids': {
        if (line.lineIndex > stopEndIndex && line.pids.length > 0) {
          fails.push(`PIDs seen after stop: ${line.pids.join(", ")}`);
        }
        break;
      }

      }
    }
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    return [1, 1];
  };
}

Object.assign(testcases, {
  '3a1': makeEx3a('3a1'),
  '3a2': makeEx3a('3a2')
});

// EXERCISE 3B

function makeEx3b(title) {
  return t => {
    const fails = ex2CheckExecs(t);
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    const waitCommandIndex = t.commands.findIndex(x => x.startsWith("wait"));
    if (waitCommandIndex === -1) {
      throw new Error("No wait command?");
    }
    const children = {};
    for (const line of t.trace) {
      switch (line.type) {
      case 'clone':
      case 'execve': {
        children[line.pid] = line;
        line.killed = false;
        line.exited = false;
        break;
      }

      case 'kill': {
        fails.push("Kill was used");
        break;
      }

      case 'sigchld': {
        const child = children[line.pid];
        if (child) {
          child.exited = true;
        }
        break;
      }

      case 'commandEnd': {
        if (line.index === waitCommandIndex) {
          for (const child of Object.values(children)) {
            if (!child.exited) {
              fails.push(`Wait returned before child ${child.path} PID ${child.pid} exited`);
            }
          }
        }
        break;
      }

      }
    }
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    return [1, 1];
  };
}

Object.assign(testcases, {
  '3b1': makeEx3b('3b1'),
  '3b2': makeEx3b('3b2')
});

// EXERCISE 3C

function makeEx3c(title) {
  return t => {
    const fails = ex2CheckExecs(t);
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }

    const children = Object.fromEntries(t.trace.filter(t => ['execve', 'clone'].includes(t.type)).map(t => [t.pid, t]));
    for (const line of t.trace.filter(t => t.type === 'kill')) {
      const child = children[line.pid];
      if (!child) {
        fails.push(`Wrong PID ${line.pid} killed`);
      }
    }

    const lastPids = t.trace.findLast(t => t.type === 'pids').pids;
    for (const lastPid of lastPids) {
      if (!children.hasOwnProperty(lastPid)) { continue; }
      const child = children[lastPid];
      fails.push(`Child ${child.path} PID ${child.pid} remaining after shutdown`);
    }

    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    return [1, 1];
  };
}

Object.assign(testcases, {
  '3c1': makeEx3c('3c1'),
  '3c2': makeEx3c('3c2')
});

// EXERCISE 4

function makeEx4(title) {
  return t => {
    const fails = [];
    for (const log of Object.values(t.expectedLogs)) {
      if (arrayEqual(t.logs[log.index].lines, log.lines)) { continue; }
      fails.push(`Log for service ${log.index} has wrong contents or does not exist`);
    }
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    return [1, 1];
  };
}

for (const i of [1, 2, 3, 4].map(i => `4_${i}`)) {
  testcases[i] = makeEx4(i);
}

// EXERCISE 5

function makeEx5(title) {
  return t => {
    const fails = [];
    for (const log of Object.values(t.expectedLogs)) {
      const showLogCommand = `showlog ${log.index}`;
      const commandIndex = t.commands.findIndex(x => x === showLogCommand);
      if (commandIndex === -1) { throw new Error("Could not find showlog command"); }
      const commandBeginLine = t.trace.findIndex(l => l.type === 'command' && l.command === showLogCommand);
      let commandEndLine = t.trace.findIndex(l => l.type === 'commandEnd' && l.index === commandIndex);
      if (commandBeginLine === -1) { throw new Error("Could not find showlog begin"); }
      if (commandEndLine === -1) {
        // maybe the testee segfaulted..
        // use the first expected log entry as the end then
        commandEndLine = t.trace.findIndex(l => l.type === 'logExpectedBegin');
      }

      const firstLogLine = log.lines[0];
      let traceIndex = t.trace.findIndex(l => l.raw.trim() === firstLogLine && l.lineIndex > commandBeginLine && l.lineIndex < commandEndLine);
      if (traceIndex === -1) {
        fails.push(`Could not find output for showlog ${log.index}`);
        continue;
      }
      for (const expLine of log.lines) {
        let found = false;
        while (traceIndex < commandEndLine) {
          let line = t.trace[traceIndex++];
          if (line.raw.trim() === expLine) { found = true; break; }
        }
        if (!found) {
          fails.push(`Wrong output for showlog ${log.index}`);
          break;
        }
      }
    }
    if (fails.length > 0) {
      return [0, 1, joinComments(title, fails)];
    }
    return [1, 1];
  };
}

for (const i of [1, 2, 3].map(i => `5_${i}`)) {
  testcases[i] = makeEx5(i);
}
