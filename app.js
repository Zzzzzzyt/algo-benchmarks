let results_index = [];

fetch("results.json")
  .then((r) => {
    if (r.ok) {
      results_index.push({ path: "results.json", name: "Default Results" });
    }
  })
  .then(() => {
    return fetch("results_index.json")
      .then((r) => r.json())
      .then((data) => {
        results_index = results_index.concat(data);
      })
      .catch((err) => {
        console.error("Failed to load results_index.json:", err);
      });
  })
  .finally(() => {
    populateProfileDropdown(results_index);
    if (results_index.length > 0) {
      loadProfile(results_index[0].path, results_index[0].name);
    }
  });

function populateProfileDropdown(profiles) {
  const menu = document.getElementById("profileMenu");
  const dropdown = document.getElementById("profileDropdown");

  menu.innerHTML = "";
  profiles.forEach((profile) => {
    const item = document.createElement("div");
    item.className = "item";
    item.setAttribute("data-value", profile.path);
    item.textContent = profile.name;
    menu.appendChild(item);
  });

  $(dropdown).dropdown({
    onChange: function (value, text) {
      loadProfile(value, text);
    },
  });

  dropdown.querySelector(".default.text").textContent = "Select a profile";
  if (document.getElementById("metricSelect")) {
    $("#metricSelect").dropdown();
  }
}

function loadProfile(path, name) {
  fetch(path)
    .then((r) => r.json())
    .then((data) => {
      window._resultsData = data.results;

      buildTree(data.results);
      displayProfileInfo(data);

      const dropdown = document.getElementById("profileDropdown");
      dropdown.querySelector(".text").textContent = name;

      refresh();
    })
    .catch((err) => {
      console.error("Failed to load profile:", err);
    });
}

function displayProfileInfo(data) {
  const infoDiv = document.getElementById("profileInfo");
  const detailsDiv = document.getElementById("profileDetails");

  infoDiv.style.display = "block";

  let html = "";

  if (data.profile) {
    const comment = htmlEscape(data.comment || data.profile.comment || "").replace(/(?:\r\n|\r|\n)/g, "<br>");
    html += `<div class="ui segment">
        <h5 class="ui header">Profile</h5>
        <p><strong>Name:</strong> ${data.profile.name || "N/A"}</p>
        <p><strong>Build Command:</strong> <code>${data.profile.build_command || "N/A"}</code></p>
        <p><strong>Comment:</strong><br>${comment || "N/A"}</p>
      </div>`;
  }

  if (data.environment) {
    const env = data.environment;
    html += `<div class="ui segment">
        <h5 class="ui header">General Information</h5>
        <p><strong>Platform:</strong> ${env.platform || "N/A"}</p>
        <p><strong>Python Version:</strong> ${env.python_version || "N/A"}</p>
        <p><strong>Timestamp:</strong> ${env.timestamp || "N/A"}</p>
        <p><strong>TSC Frequency:</strong> ${env.tsc_freq ? (env.tsc_freq * 1000).toFixed(3) + "MHz" : "N/A"}</p>
      </div>`;
    if (env.sysinfo) {
      html += `<div class="ui segment">
            <h5 class="ui header">System Information</h5>
            <pre style="text-wrap:auto;">${env.sysinfo}</pre>
          </div>`;
    }
    if (env.cacheinfo) {
      html += `<div class="ui segment">
            <h5 class="ui header">Cache Information</h5>
            <pre style="text-wrap:auto;">${env.cacheinfo}</pre>
          </div>`;
    }
    if (env.meminfo) {
      html += `<div class="ui segment">
            <h5 class="ui header">Memory Information</h5>
            <pre style="text-wrap:auto;">${env.meminfo}</pre>
          </div>`;
    }
    if (env.cpuinfo) {
      html += `<div class="ui segment">
            <h5 class="ui header">CPU Information</h5>
            <pre style="text-wrap:auto;">${env.cpuinfo}</pre>
          </div>`;
    }
    if (env["g++"]) {
      html += `<div class="ui segment">
            <h5 class="ui header">Compiler</h5>
            <pre style="text-wrap:auto;">${env["g++"]}</pre>
          </div>`;
    }
  }

  detailsDiv.innerHTML = html;
  $(infoDiv).find(".ui.accordion").accordion();
}

function buildTree(results) {
  function buildNestedTree(keys) {
    const root = new Map();
    for (const key of keys) {
      const parts = key.split(".");
      let node = root;
      for (let i = 0; i < parts.length - 1; ++i) {
        if (!node.has(parts[i])) {
          node.set(parts[i], new Map());
        }
        node = node.get(parts[i]);
      }
      node.set(parts[parts.length - 1], key);
    }
    return root;
  }

  function renderTree(node) {
    let html = "";
    for (const [k, v] of node.entries()) {
      if (typeof v === "string") {
        html += `<div class="ui checkbox scenario-checkbox"><input type="checkbox" id="data-key-${v}"><label for="data-key-${v}">${k}</label></div>`;
      } else {
        html += `<div class="ui accordion"><div class="title"><i class="dropdown icon"></i>${k}</div>`;
        html += '<div class="content">' + renderTree(v) + "</div></div>";
      }
    }
    return html;
  }

  const tree = document.getElementById("tree");
  tree.innerHTML = renderTree(buildNestedTree(Object.keys(results)));

  $("#tree>.accordion").accordion({ exclusive: false, duration: 100 });

  const checkboxes = tree.querySelectorAll("input[type=checkbox]");
  checkboxes.forEach((cb) => {
    cb.onchange = () => {
      refresh();
    };
  });
}

function getMetricAccessors(metric) {
  if (metric === "time_ns") {
    return {
      meanKeys: ["time_ns_mean", "mean"],
      stdKeys: ["time_ns_stddev", "stddev"],
      minKeys: ["time_ns_min", "min"],
      maxKeys: ["time_ns_max", "max"],
    };
  }
  if (metric === "constant") {
    return {
      meanKeys: ["constant_mean", "mean_c"],
      stdKeys: ["constant_stddev", "stddev_c"],
      minKeys: ["constant_min", "min_c"],
      maxKeys: ["constant_max", "max_c"],
    };
  }
  return {
    meanKeys: [`${metric}_mean`, metric],
    stdKeys: [`${metric}_stddev`],
    minKeys: [`${metric}_min`],
    maxKeys: [`${metric}_max`],
  };
}

function getFirstFiniteValue(obj, keys) {
  for (const key of keys) {
    if (obj[key] !== undefined && obj[key] !== null) {
      const num = Number(obj[key]);
      if (Number.isFinite(num)) {
        return num;
      }
    }
  }
  return undefined;
}

function showOverlayPlot(resultsArr, keysArr) {
  const plotDiv = document.getElementById("plot");
  const detailsDiv = document.getElementById("details");
  const metric = document.getElementById("metricSelect")?.value || "time_ns";
  const use_c = metric === "constant";
  if (!resultsArr.length) {
    plotDiv.innerHTML = "";
    detailsDiv.style.display = "none";
    return;
  }

  let xType = "log";
  let yType = "log";

  const showLines = document.getElementById("linesToggle")?.checked;
  const showMinMax = document.getElementById("minmaxToggle")?.checked;
  const logRegex = /O\((log n|logn)\)/i;
  if (use_c || metric !== "time_ns") {
    yType = "linear";
  } else if (resultsArr.length > 0) {
    const allLogLinear = resultsArr.every((r) => logRegex.test(r.complexity));
    if (allLogLinear) {
      yType = "linear";
    }
  }

  const traces = [];
  const colorScheme = [
    "#1f77b4",
    "#ff7f0e",
    "#2ca02c",
    "#d62728",
    "#9467bd",
    "#8c564b",
    "#e377c2",
    "#7f7f7f",
    "#bcbd22",
    "#17becf",
  ];

  resultsArr.forEach((result, idx) => {
    const xs = [];
    const ys = [];
    const errors = [];
    const errors2 = [];
    const access = getMetricAccessors(metric);

    result.stats.forEach((d) => {
      const n = Number(d.n);
      const meanVal = getFirstFiniteValue(d, access.meanKeys);
      if (!Number.isFinite(meanVal)) {
        return;
      }
      const stdVal = getFirstFiniteValue(d, access.stdKeys);
      const minVal = getFirstFiniteValue(d, access.minKeys);
      const maxVal = getFirstFiniteValue(d, access.maxKeys);

      xs.push(n);
      ys.push(meanVal);

      if (showMinMax) {
        if (Number.isFinite(minVal) && Number.isFinite(maxVal)) {
          errors2.push(Math.max(0, meanVal - minVal));
          errors.push(Math.max(0, maxVal - meanVal));
        } else if (Number.isFinite(stdVal)) {
          errors2.push(stdVal);
          errors.push(stdVal);
        } else {
          errors2.push(0);
          errors.push(0);
        }
      } else if (Number.isFinite(stdVal)) {
        errors.push(stdVal);
      } else {
        errors.push(0);
      }
    });

    if (xs.length === 0) {
      return;
    }

    traces.push({
      x: xs,
      y: ys,
      error_y: showMinMax
        ? { type: "data", symmetric: false, array: errors, arrayminus: errors2, visible: true }
        : { type: "data", array: errors, visible: true },
      mode: showLines ? "lines+markers" : "markers",
      type: "scatter",
      name: keysArr[idx],
      marker: { color: colorScheme[idx % colorScheme.length] },
    });
  });

  const metricLabel = document.querySelector(`#metricSelect option[value="${metric}"]`)?.textContent || metric;
  const yTitle = use_c ? "Time / Complexity" : metricLabel;
  const plotLayout = {
    title: keysArr.join(" + "),
    xaxis: { title: "Input size (n)", type: xType },
    yaxis: {
      title: yTitle,
      type: yType,
      rangemode: yType === "log" ? undefined : "tozero",
    },
    legend: { orientation: "h" },
    margin: { t: 40 },
  };

  Plotly.newPlot(plotDiv, traces, plotLayout, { responsive: true });
  detailsDiv.style.display = "";
  detailsDiv.innerHTML = resultsArr
    .map((result, idx) => {
      const constantMax = Number(result.constant_max ?? result.max_c);
      const constantText = Number.isFinite(constantMax) ? constantMax.toFixed(3) : "N/A";
      return [
        `<b>Algorithm:</b> ${keysArr[idx]}`,
        `<b>Type:</b> ${result.type}`,
        `<b>Complexity:</b> ${result.complexity}`,
        `<b>Max Constant:</b> ${constantText}`,
        `<b>Description:</b><br>${htmlEscape(result.description_en || "N/A")}`,
      ].join("<br>");
    })
    .join("<hr>");
}

function refresh() {
  const tree = document.getElementById("tree");
  if (!tree) {
    return;
  }
  const checked = Array.from(tree.querySelectorAll("input[type=checkbox]:checked"));
  const selectedKeys = checked.map((c) => c.id.replace("data-key-", ""));
  if (window._resultsData) {
    const selectedEntries = selectedKeys
      .map((k) => ({ key: k, result: window._resultsData[k] }))
      .filter((entry) => Boolean(entry.result));
    showOverlayPlot(
      selectedEntries.map((entry) => entry.result),
      selectedEntries.map((entry) => entry.key)
    );
  }
}

function htmlEscape(str) {
  if (str === undefined || str === null) {
    return "";
  }
  return String(str)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}
