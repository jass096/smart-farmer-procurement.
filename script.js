
const STORAGE_KEY = "kisanQueueFarmers";
const TOKEN_KEY = "kisanQueueNextToken";

let farmers = [];
let nextToken = 1;
let toastTimer;

const statuses = [
  "Waiting",
  "Weighing",
  "Accepted",
  "Payment Pending",
  "Paid",
  "Rejected"
];

const $ = (id) => document.getElementById(id);

// Escape user-entered text before displaying it in HTML.
function escapeHTML(value) {
  return String(value).replace(/[&<>"']/g, char => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#39;"
  })[char]);
}

function loadData() {
  try {
    const saved = JSON.parse(localStorage.getItem(STORAGE_KEY) || "[]");
    farmers = Array.isArray(saved) ? saved : [];
  } catch {
    farmers = [];
  }

  const savedNext = Number(localStorage.getItem(TOKEN_KEY)) || 1;
  const highestToken = farmers.reduce((highest, farmer) => {
    const match = /^T(\d+)$/.exec(farmer.token || "");
    return match ? Math.max(highest, Number(match[1]) + 1) : highest;
  }, 1);

  nextToken = Math.max(savedNext, highestToken);
}

function saveData() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(farmers));
    localStorage.setItem(TOKEN_KEY, String(nextToken));
    return true;
  } catch {
    showToast("Could not save data. Check browser storage.");
    return false;
  }
}

function showToast(message) {
  const toast = $("toast");
  toast.textContent = message;
  toast.classList.add("show");

  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => {
    toast.classList.remove("show");
  }, 2800);
}

function findFarmer(query) {
  const value = query.trim().toLowerCase();

  return farmers.find(f =>
    f.id.toLowerCase() === value ||
    f.token.toLowerCase() === value
  );
}

function statusBadge(status) {
  const className = "status-" + status
    .toLowerCase()
    .replace(/\s+/g, "-");

  return `<span class="status ${className}">${escapeHTML(status)}</span>`;
}

function renderDashboard() {
  $("totalCount").textContent = farmers.length;
  $("waitingCount").textContent =
    farmers.filter(f => f.status === "Waiting").length;
  $("acceptedCount").textContent =
    farmers.filter(f => f.status === "Accepted").length;
  $("paymentCount").textContent =
    farmers.filter(f => f.status === "Payment Pending").length;

  const recent = [...farmers].reverse().slice(0, 5);

  $("recentTable").innerHTML = recent.length
    ? recent.map(f => `
      <tr>
        <td><strong>${escapeHTML(f.token)}</strong><br>
            <small>${escapeHTML(f.id)}</small></td>
        <td>${escapeHTML(f.name)}</td>
        <td>${escapeHTML(f.crop)}</td>
        <td>${escapeHTML(f.slot)}</td>
        <td>${statusBadge(f.status)}</td>
      </tr>
    `).join("")
    : `<tr><td colspan="5" class="empty-row">
        No farmer records yet. Register your first farmer.
       </td></tr>`;
}

function renderQueue() {
  const search = $("queueSearch").value.trim().toLowerCase();
  const filter = $("statusFilter").value;

  const result = farmers.filter(f => {
    const matchesSearch = [
      f.id, f.token, f.name, f.village, f.crop
    ].some(value => value.toLowerCase().includes(search));

    return matchesSearch && (!filter || f.status === filter);
  });

  $("queueTable").innerHTML = result.length
    ? result.map(f => `
      <tr>
        <td><strong>${escapeHTML(f.token)}</strong></td>
        <td>${escapeHTML(f.name)}<br>
            <small>${escapeHTML(f.id)}</small></td>
        <td>${escapeHTML(f.crop)}<br>
            <small>${escapeHTML(f.quantity)} kg</small></td>
        <td>${escapeHTML(f.slot)}</td>
        <td>${statusBadge(f.status)}</td>
        <td>
          <select aria-label="Update status for ${escapeHTML(f.id)}"
                  data-status-id="${escapeHTML(f.id)}">
            ${statuses.map(status => `
              <option value="${escapeHTML(status)}"
                ${status === f.status ? "selected" : ""}>
                ${escapeHTML(status)}
              </option>
            `).join("")}
          </select>
        </td>
      </tr>
    `).join("")
    : `<tr><td colspan="6" class="empty-row">
        No matching farmer records found.
       </td></tr>`;
}

function renderAll() {
  renderDashboard();
  renderQueue();
}

function showPage(pageId) {
  document.querySelectorAll(".page").forEach(page => {
    page.classList.toggle("hidden", page.id !== pageId);
  });

  document.querySelectorAll(".nav-btn").forEach(button => {
    button.classList.toggle("active", button.dataset.page === pageId);
  });

  const titles = {
    dashboard: "Dashboard",
    register: "Register Farmer",
    queue: "Queue Management",
    track: "Track Procurement"
  };

  $("pageTitle").textContent = titles[pageId] || "Dashboard";

  if (pageId === "dashboard") renderDashboard();
  if (pageId === "queue") renderQueue();

  window.scrollTo({ top: 0, behavior: "smooth" });
}

// Sidebar navigation
document.querySelectorAll(".nav-btn").forEach(button => {
  button.addEventListener("click", () => showPage(button.dataset.page));
});

document.querySelectorAll("[data-go]").forEach(button => {
  button.addEventListener("click", () => showPage(button.dataset.go));
});

// Register a farmer and assign a slot token
$("farmerForm").addEventListener("submit", event => {
  event.preventDefault();

  const id = $("farmerId").value.trim();

  if (farmers.some(f => f.id.toLowerCase() === id.toLowerCase())) {
    showToast("This Farmer ID already exists.");
    return;
  }

  const quantity = Number($("quantity").value);

  if (!Number.isFinite(quantity) || quantity <= 0) {
    showToast("Enter a valid crop quantity.");
    return;
  }

  const farmer = {
    id,
    name: $("farmerName").value.trim(),
    village: $("village").value.trim(),
    crop: $("crop").value,
    quantity,
    slot: $("slot").value,
    token: "T" + String(nextToken).padStart(3, "0"),
    status: "Waiting"
  };

  if (!farmer.name || !farmer.village ||
      !farmer.crop || !farmer.slot) {
    showToast("Please complete all fields.");
    return;
  }

  farmers.push(farmer);
  nextToken++;

  if (!saveData()) {
    farmers.pop();
    nextToken--;
    return;
  }

  $("farmerForm").reset();
  renderAll();
  showPage("dashboard");
  showToast("Farmer registered! Token: " + farmer.token);
});

// Queue search and filters
$("queueSearch").addEventListener("input", renderQueue);
$("statusFilter").addEventListener("change", renderQueue);

// Update procurement status
$("queueTable").addEventListener("change", event => {
  const select = event.target.closest("select[data-status-id]");
  if (!select) return;

  const farmer = farmers.find(f => f.id === select.dataset.statusId);
  if (!farmer) return;

  const oldStatus = farmer.status;
  farmer.status = select.value;

  if (!saveData()) {
    farmer.status = oldStatus;
    renderQueue();
    return;
  }

  renderAll();
  showToast("Procurement status updated.");
});

// Track a farmer by ID or token
$("trackForm").addEventListener("submit", event => {
  event.preventDefault();

  const query = $("trackInput").value;
  const farmer = findFarmer(query);
  const result = $("trackResult");

  if (!farmer) {
    result.innerHTML = `
      <div class="info-box">
        No farmer found. Check the Farmer ID or queue token.
      </div>`;
    return;
  }

  result.innerHTML = `
    <div class="track-result-card">
      <h4>Procurement Details</h4>
      <p><strong>Farmer:</strong> ${escapeHTML(farmer.name)}</p>
      <p><strong>Farmer ID:</strong> ${escapeHTML(farmer.id)}</p>
      <p><strong>Village:</strong> ${escapeHTML(farmer.village)}</p>
      <p><strong>Crop:</strong> ${escapeHTML(farmer.crop)}</p>
      <p><strong>Quantity:</strong> ${escapeHTML(farmer.quantity)} kg</p>
      <p><strong>Queue Token:</strong> ${escapeHTML(farmer.token)}</p>
      <p><strong>Slot:</strong> ${escapeHTML(farmer.slot)}</p>
      <p><strong>Status:</strong> ${statusBadge(farmer.status)}</p>
      <button type="button" class="primary-btn" id="viewQueueButton">
        View Queue
      </button>
    </div>`;

  $("viewQueueButton").addEventListener("click", () => {
    showPage("queue");
    $("queueSearch").value = farmer.id;
    renderQueue();
  });
});

// Initial setup
loadData();
renderAll();
