#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

function getBuildDir() {
  if (process.env.SOAR_BUILD_DIR) {
    return process.env.SOAR_BUILD_DIR;
  }

  const scriptDir = __dirname;
  const candidates = [
    path.join(scriptDir, "..", "build", "Release"),
    path.join(scriptDir, "..", "build", "Debug"),
  ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return path.join(scriptDir, "..", "build", "Debug");
}

function loadBindings(buildDir) {
  const candidates = [
    path.join(buildDir, "Core", "ClientSMLSWIG", "javascript", "JavaScript_sml_ClientInterface.js"),
    path.join(buildDir, "Core", "ClientSMLSWIG", "JavaScript_sml_ClientInterface.js"),
    path.join(buildDir, "Core", "ClientSMLSWIG", "javascript", "JavaScript_sml_ClientInterface.node"),
    path.join(buildDir, "Core", "ClientSMLSWIG", "JavaScript_sml_ClientInterface.node"),
    path.join(__dirname, "..", "Core", "ClientSMLSWIG", "JS", "JavaScript_sml_ClientInterface.js"),
  ];

  for (const candidate of candidates) {
    if (!fs.existsSync(candidate)) {
      continue;
    }

    try {
      return require(candidate);
    } catch (error) {
      console.log(`[WARN] Failed loading candidate ${candidate}: ${error.message}`);
    }
  }

  throw new Error(`Unable to load JavaScript SWIG bindings. Checked: ${candidates.join(", ")}`);
}

function main() {
  const buildDir = getBuildDir();
  console.log(`Build directory: ${buildDir}`);

  const bindings = loadBindings(buildDir);
  const Kernel = bindings && bindings.Kernel;

  if (!Kernel) {
    throw new Error(`Kernel class not found in bindings. Top-level keys: ${Object.keys(bindings || {}).join(", ")}`);
  }

  const createKernel = Kernel.CreateKernelInNewThread || Kernel.CreateKernelInCurrentThread;
  if (typeof createKernel !== "function") {
    throw new Error("No kernel factory function found (CreateKernelInNewThread/CreateKernelInCurrentThread)");
  }

  let kernel = null;
  let agent = null;

  try {
    kernel = createKernel.call(Kernel);
    if (!kernel) {
      throw new Error("Kernel creation returned null/undefined");
    }

    if (typeof kernel.CreateAgent !== "function") {
      throw new Error("CreateAgent method not found on kernel");
    }

    agent = kernel.CreateAgent("JavaScriptSoarTestAgent");
    if (!agent) {
      throw new Error("Agent creation returned null/undefined");
    }

    if (typeof agent.GetAgentName === "function") {
      const agentName = agent.GetAgentName();
      console.log(`Agent created: ${agentName}`);
    } else {
      console.log("Agent created (GetAgentName unavailable in this binding)");
    }

    console.log("JavaScript kernel+agent test succeeded");
  } finally {
    if (kernel && agent && typeof kernel.DestroyAgent === "function") {
      try {
        kernel.DestroyAgent(agent);
      } catch (_) {
      }
    }

    if (kernel && typeof kernel.Shutdown === "function") {
      try {
        kernel.Shutdown();
      } catch (_) {
      }
    }
  }
}

main();
