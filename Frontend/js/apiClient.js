// Part 0: Shared HTTP API client with Bearer token injection and 30-minute session timeout handling
const API_BASE = "/api";

async function apiRequest(endpoint, options = {}) {
    const token = localStorage.getItem("access_token");
    const headers = Object.assign({ "Content-Type": "application/json" }, options.headers || {});
    if (token) headers["Authorization"] = "Bearer " + token;

    const response = await fetch(API_BASE + endpoint, { ...options, headers });
    if (response.status === 401 && !endpoint.includes("/auth/login")) {
        localStorage.clear();
        window.location.href = "/index.html";
        return null;
    }

    const contentType = response.headers.get("content-type") || "";
    const data = contentType.includes("application/json") ? await response.json() : await response.text();
    if (!response.ok) {
        throw new Error((data && data.error) ? data.error : "Request failed");
    }
    return data;
}
