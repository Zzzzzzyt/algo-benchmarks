let results_index = [];

const msgpack = window.msgpack;

fetch("results.msgpack")
  .then((r) => {
    if (r.ok) {
      results_index.push({ path: "results.msgpack", name: "Default Results" });
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
    $("#metricSelect").dropdown({
      onChange: function () {
        onMetricChange();
      },
    });
  }
}

function getSelectedEntries() {
  const tree = document.getElementById("tree");
  if (!tree || !window._resultsData) {
    return [];
  }

  return Array.from(tree.querySelectorAll("input[type=checkbox]:checked"))
    .map((checkbox) => checkbox.id.replace("data-key-", ""))
    .map((key) => ({ key, result: window._resultsData[key] }))
    .filter((entry) => Boolean(entry.result));
}

function loadProfile(path, name) {
  fetch(path)
    .then((r) => r.arrayBuffer())
    .then((raw) => {
      const data = msgpack.decode(new Uint8Array(raw));
      window._resultsData = data.results;

      buildTree(data.results);
      displayProfileInfo(data);

      const dropdown = document.getElementById("profileDropdown");
      dropdown.querySelector(".text").textContent = name;

      updateAxisScaleControls();
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

function getDefaultAxisTypes(metric, resultsArr) {
  let xType = "log";
  let yType = "log";
  const logRegex = /O\((log n|logn)\)/i;

  if (metric === "constant" || /rate/i.test(metric)) {
    yType = "linear";
  } else if (resultsArr.length > 0) {
    const allLogLinear = resultsArr.every((result) => logRegex.test(result.complexity));
    if (allLogLinear) {
      yType = "linear";
    }
  }

  return { xType, yType };
}

function setAxisScale(axis, scale) {
  const buttons = document.querySelectorAll(`.axis-scale-button[data-axis="${axis}"]`);
  buttons.forEach((button) => {
    const isActive = button.dataset.scale === scale;
    button.classList.toggle("active", isActive);
    button.setAttribute("aria-pressed", isActive ? "true" : "false");
  });
}

function getAxisScale(axis) {
  const activeButton = document.querySelector(`.axis-scale-button.active[data-axis="${axis}"]`);
  return activeButton?.dataset.scale || "log";
}

function updateAxisScaleControls(metric = document.getElementById("metricSelect")?.value || "time_clock") {
  const selectedResults = getSelectedEntries().map((entry) => entry.result);
  const { xType, yType } = getDefaultAxisTypes(metric, selectedResults);
  setAxisScale("x", xType);
  setAxisScale("y", yType);
}

function onMetricChange() {
  updateAxisScaleControls();
  refresh();
}

function initializeAxisScaleControls() {
  document.querySelectorAll(".axis-scale-button").forEach((button) => {
    button.addEventListener("click", () => {
      setAxisScale(button.dataset.axis, button.dataset.scale);
      refresh();
    });
  });
}

function showOverlayPlot(resultsArr, keysArr) {
  const plotDiv = document.getElementById("plot");
  const detailsDiv = document.getElementById("details");
  const metric = document.getElementById("metricSelect")?.value || "time_clock";
  if (!resultsArr.length) {
    plotDiv.innerHTML = "";
    detailsDiv.style.display = "none";
    return;
  }

  const xType = getAxisScale("x");
  const yType = getAxisScale("y");
  const showLines = document.getElementById("linesToggle")?.checked;
  const showMinMax = document.getElementById("minmaxToggle")?.checked;

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

    const data = new DataView(result.stats.data.buffer);
    const data_keys = result.stats.keys;
    const entryCount = data.byteLength / (data_keys.length * 4);
    const keyIndexN = data_keys.indexOf("n");
    const keyIndexMean = data_keys.indexOf(`${metric}_mean`);
    const keyIndexStddev = data_keys.indexOf(`${metric}_stddev`);
    const keyIndexMin = data_keys.indexOf(`${metric}_min`);
    const keyIndexMax = data_keys.indexOf(`${metric}_max`);

    function getValue(entryIdx, keyIdx) {
      return data.getFloat32((entryIdx * data_keys.length + keyIdx) * 4, true);
    }

    for (let i = 0; i < entryCount; i++) {
      const n = getValue(i, keyIndexN);
      console.log(n);
      const meanVal = getValue(i, keyIndexMean);
      if (!Number.isFinite(meanVal)) {
        return;
      }
      const stdVal = getValue(i, keyIndexStddev);
      const minVal = getValue(i, keyIndexMin);
      const maxVal = getValue(i, keyIndexMax);

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
    }

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
  const yTitle = metricLabel;
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
  if (window._resultsData) {
    const selectedEntries = getSelectedEntries();
    showOverlayPlot(
      selectedEntries.map((entry) => entry.result),
      selectedEntries.map((entry) => entry.key),
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

initializeAxisScaleControls();
