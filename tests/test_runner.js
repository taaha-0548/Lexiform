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
    expectedProps: ["title", "id", "sections"]
  },
  {
    name: "Survey Complex",
    file: "survey_complex.form",
    shouldPass: true,
    expectedProps: ["title", "id", "sections"]
  },
  {
    name: "Semantic Error (Attribute Mismatch)",
    file: "error_semantic.form",
    shouldPass: false,
    expectedError: "Semantic Error"
  }
];

let total = 0;
let passed = 0;

console.log("🚀 Lexiform Compiler Test Suite (JS Runner)\n");

if (!fs.existsSync(COMPILER_PATH)) {
  console.error(`❌ ERROR: Compiler not found at ${COMPILER_PATH}`);
  console.error("Please run 'npm run build:core' first.");
  process.exit(1);
}

scenarios.forEach((scenario) => {
  total++;
  const inputPath = path.join(SCENARIOS_DIR, scenario.file);
  const outputPath = inputPath.replace('.form', '.json');

  try {
    console.log(`[TEST] ${scenario.name}...`);

    // Attempt compilation
    execSync(`${COMPILER_PATH} ${inputPath} -o ${outputPath}`, { stdio: 'pipe' });

    if (scenario.shouldPass) {
      // Validate JSON content
      const json = JSON.parse(fs.readFileSync(outputPath, 'utf8'));        
      const isValid = scenario.expectedProps.every(prop => prop in json);  

      if (isValid) {
        console.log(` ✅ SUCCESS: JSON generated and validated.\n`);      
        passed++;
      } else {
        console.log(` ❌ FAILURE: JSON missing expected properties.\n`);   
      }
    } else {
      console.log(` ❌ FAILURE: Expected compilation to fail, but it succeeded.\n`);
    }

  } catch (error) {
    const errorMsg = error.stderr ? error.stderr.toString() : error.message;

    if (!scenario.shouldPass && errorMsg.toLowerCase().includes(scenario.expectedError.toLowerCase())) {
      console.log(` ✅ SUCCESS: Correctly identified semantic error: "${scenario.expectedError}"\n`);
      passed++;
    } else if (scenario.shouldPass) {
      console.log(` ❌ FAILURE: Compilation should have passed but failed with error: ${errorMsg}\n`);
    } else {
      console.log(` ❌ FAILURE: Unexpected error message: ${errorMsg}\n`);
    }
  } finally {
    if (fs.existsSync(outputPath)) fs.unlinkSync(outputPath);
  }
});

console.log(`\n📊 FINAL RESULT: ${passed}/${total} Scenarios Passed.`);  

if (passed === total) {
  process.exit(0);
} else {
  process.exit(1);
}
