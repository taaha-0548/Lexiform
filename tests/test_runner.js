const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

// Determine executable extension based on platform
const isWindows = process.platform === 'win32';
const exeExt = isWindows ? '.exe' : '';
const COMPILER_PATH = path.resolve(__dirname, `../build/lexiform${exeExt}`);
const SCENARIOS_DIR = path.resolve(__dirname, 'scenarios');

const scenarios = [
  {
    name: "Contact Simple",
    file: "contact_simple.form",
    shouldPass: true,
    expectedProps: ["title", "id", "sections", "submit"]
  },
  {
    name: "Survey Complex (Multi-section, MIN/MAX, OPTIONS)",
    file: "survey_complex.form",
    shouldPass: true,
    expectedProps: ["title", "id", "sections", "submit"]
  },
  {
    name: "Optimization Demo (Triggers both optimizer passes)",
    file: "optimization_demo.form",
    shouldPass: true,
    expectedProps: ["title", "id", "sections", "submit"]
  },
  {
    name: "Semantic Error (Attribute Mismatch — MAX_WORDS on TEXT)",
    file: "error_semantic.form",
    shouldPass: false,
    expectedError: "Semantic Error"
  }
];

// Additional standalone test files (outside scenarios/)
const standaloneTests = [
  {
    name: "Job Application Form",
    file: path.resolve(__dirname, "job_app.form"),
    shouldPass: true,
    expectedProps: ["title", "id", "sections", "submit"]
  },
  {
    name: "YouTube Upload Form",
    file: path.resolve(__dirname, "youtube_upload.form"),
    shouldPass: true,
    expectedProps: ["title", "id", "sections", "submit"]
  },
  {
    name: "Duplicate ID Error",
    file: path.resolve(__dirname, "invalid_duplicate_id.form"),
    shouldPass: false,
    expectedError: "Duplicate ID"
  },
  {
    name: "Attribute Mismatch Error",
    file: path.resolve(__dirname, "invalid_attr_mismatch.form"),
    shouldPass: false,
    expectedError: "Semantic Error"
  }
];

let total = 0;
let passed = 0;

console.log("🚀 Lexiform Compiler Test Suite (Full Runner)\n");
console.log(`Compiler: ${COMPILER_PATH}\n`);

if (!fs.existsSync(COMPILER_PATH)) {
  console.error(`❌ ERROR: Compiler not found at ${COMPILER_PATH}`);
  console.error("Please run 'npm run build:core' first.");
  process.exit(1);
}

function runTest(scenario, inputPath) {
  total++;
  const outputPath = inputPath.replace('.form', '_output.json');

  try {
    console.log(`[TEST ${total}] ${scenario.name}...`);

    // Attempt compilation
    execSync(`"${COMPILER_PATH}" "${inputPath}" -o "${outputPath}"`, { stdio: 'pipe' });

    if (scenario.shouldPass) {
      // Validate JSON content
      const json = JSON.parse(fs.readFileSync(outputPath, 'utf8'));
      const isValid = scenario.expectedProps.every(prop => prop in json);

      if (isValid) {
        console.log(` ✅ SUCCESS: JSON generated and validated.`);
        passed++;
      } else {
        const missing = scenario.expectedProps.filter(p => !(p in json));
        console.log(` ❌ FAILURE: JSON missing properties: ${missing.join(', ')}`);
      }
    } else {
      console.log(` ❌ FAILURE: Expected compilation to fail, but it succeeded.`);
    }

  } catch (error) {
    const errorMsg = error.stderr ? error.stderr.toString() : error.message;

    if (!scenario.shouldPass && errorMsg.toLowerCase().includes(scenario.expectedError.toLowerCase())) {
      console.log(` ✅ SUCCESS: Correctly identified error: "${scenario.expectedError}"`);
      passed++;
    } else if (scenario.shouldPass) {
      console.log(` ❌ FAILURE: Should have passed but failed: ${errorMsg}`);
    } else {
      console.log(` ❌ FAILURE: Unexpected error: ${errorMsg}`);
    }

  } finally {
    if (fs.existsSync(outputPath)) fs.unlinkSync(outputPath);
  }

  console.log('');
}

// Run scenario-based tests
console.log("── Scenario Tests ──\n");
scenarios.forEach((scenario) => {
  const inputPath = path.join(SCENARIOS_DIR, scenario.file);
  runTest(scenario, inputPath);
});

// Run standalone tests
console.log("── Standalone Tests ──\n");
standaloneTests.forEach((scenario) => {
  runTest(scenario, scenario.file);
});

// --- Debug output test (separate check) ---
console.log("── Debug Mode Test ──\n");
total++;
try {
  const debugInput = path.join(SCENARIOS_DIR, "contact_simple.form");
  const debugOutput = execSync(`"${COMPILER_PATH}" "${debugInput}" --debug -o "${path.join(SCENARIOS_DIR, 'debug_test.json')}"`, {
    stdio: 'pipe',
    encoding: 'utf8'
  });

  const stdout = debugOutput.toString ? debugOutput.toString() : debugOutput;
  const hasTokens = stdout.includes("Phase 1") || stdout.includes("Token");
  const hasAST = stdout.includes("Phase 2") || stdout.includes("AST");
  const hasIR = stdout.includes("Phase 4") || stdout.includes("IR");

  if (hasTokens && hasAST && hasIR) {
    console.log(` ✅ SUCCESS: --debug flag shows all intermediate representations.`);
    passed++;
  } else {
    console.log(` ⚠️  PARTIAL: --debug output missing some phases.`);
  }
} catch (error) {
  console.log(` ❌ FAILURE: --debug mode crashed: ${error.message}`);
} finally {
  const debugJson = path.join(SCENARIOS_DIR, 'debug_test.json');
  if (fs.existsSync(debugJson)) fs.unlinkSync(debugJson);
}

console.log(`\n📊 FINAL RESULT: ${passed}/${total} Tests Passed.`);

if (passed === total) {
  process.exit(0);
} else {
  process.exit(1);
}
