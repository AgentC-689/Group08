// Part 0: Renders role-based sidebar navigation matching the Figma design (NFR04)
function initLayout(activePage) {
    const token = localStorage.getItem("access_token");
    const role = localStorage.getItem("role");
    const username = localStorage.getItem("username");
    if (!token || !role) {
        window.location.href = "/index.html";
        return;
    }

    const links = [{ href: "/dashboard.html", label: "Dashboard", id: "dashboard" }];
    if (role === "Administrator") {
        links.push({ href: "/admin_accounts.html", label: "Manage Accounts", id: "accounts" });
        links.push({ href: "/admin_students.html", label: "Manage Students", id: "students" });
        links.push({ href: "/admin_classes.html", label: "Manage Classes", id: "classes" });
        links.push({ href: "/admin_courses.html", label: "Manage Courses", id: "courses" });
    }
    if (role === "Lecturer") {
        links.push({ href: "/lecturer_classes.html", label: "My Classes", id: "my_classes" });
        links.push({ href: "/lecturer_attendance.html", label: "Attendance", id: "attendance" });
    }
    if (role === "Student") {
        links.push({ href: "/student_checkin.html", label: "Self Check-In", id: "checkin" });
    }
    links.push({ href: "/history.html", label: "Attendance History", id: "history" });
    links.push({ href: "/reports.html", label: "Attendance Report", id: "reports" });
    links.push({ href: "/change_password.html", label: "Change Password", id: "change_password" });

    const sidebar = document.getElementById("sidebar");
    if (!sidebar) return;
    sidebar.innerHTML = `
        <div>
            <h1>STUDENT SYSTEM</h1>
            <p style="font-size:12px;color:#bfdbfe;text-align:center;margin-bottom:18px;">${username} (${role})</p>
            <nav class="nav-links">
                ${links.map(l => `<a href="${l.href}" class="${l.id === activePage ? 'active' : ''}">${l.label}</a>`).join("")}
            </nav>
        </div>
        <button class="btn-logout" onclick="logoutUser()">Logout</button>
    `;
}

async function logoutUser() {
    try { await apiRequest("/auth/logout", { method: "POST" }); } catch (_) {}
    localStorage.clear();
    window.location.href = "/index.html";
}
