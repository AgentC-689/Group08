// Part 1 (Member 1): Login, Password Change, and Account Management UI logic (FR01-FR03)
async function handleLoginSubmit(event) {
    event.preventDefault();
    const msg = document.getElementById("loginMsg");
    try {
        const data = await apiRequest("/auth/login", {
            method: "POST",
            body: JSON.stringify({
                username: document.getElementById("username").value.trim(),
                password: document.getElementById("password").value
            })
        });
        localStorage.setItem("access_token", data.access_token);
        localStorage.setItem("role", data.role);
        localStorage.setItem("username", data.username);
        window.location.href = "/dashboard.html";
    } catch (err) {
        msg.style.display = "block";
        msg.textContent = err.message;
    }
}

async function loadAccounts() {
    const tbody = document.getElementById("accountsBody");
    if (!tbody) return;
    const rows = await apiRequest("/accounts");
    tbody.innerHTML = rows.map(u => `
        <tr>
            <td>${u.user_id}</td>
            <td>${u.username}</td>
            <td>${u.role}</td>
            <td>${u.is_locked ? "Locked" : "Active"}</td>
            <td>
                <button class="btn btn-secondary" onclick="toggleLock(${u.user_id})">${u.is_locked ? "Unlock" : "Lock"}</button>
                <button class="btn" onclick="resetPass(${u.user_id})">Reset Password</button>
            </td>
        </tr>
    `).join("");
}

async function createAccount(event) {
    event.preventDefault();
    await apiRequest("/accounts", {
        method: "POST",
        body: JSON.stringify({
            username: document.getElementById("accUsername").value.trim(),
            password: document.getElementById("accPassword").value,
            role: document.getElementById("accRole").value
        })
    });
    event.target.reset();
    loadAccounts();
}

async function toggleLock(userId) {
    await apiRequest(`/accounts/${userId}/lock`, { method: "PATCH" });
    loadAccounts();
}

async function resetPass(userId) {
    const pwd = prompt("Enter new password (min 6 characters):", "123456");
    if (!pwd) return;
    await apiRequest(`/accounts/${userId}/reset-password`, {
        method: "POST",
        body: JSON.stringify({ new_password: pwd })
    });
    alert("Password has been reset.");
}
async function handleChangePassword(event) {
    event.preventDefault();
    const msg = document.getElementById("pwdMsg");
    const oldPassword = document.getElementById("oldPassword").value;
    const newPassword = document.getElementById("newPassword").value;
    const confirmPassword = document.getElementById("confirmPassword").value;

    if (newPassword !== confirmPassword) {
        msg.style.display = "block";
        msg.textContent = "New password and confirmation do not match.";
        return;
    }
    try {
        const res = await apiRequest("/auth/change-password", {
            method: "POST",
            body: JSON.stringify({ old_password: oldPassword, new_password: newPassword })
        });
        msg.style.display = "block";
        msg.textContent = res.message;
        event.target.reset();
    } catch (err) {
        msg.style.display = "block";
        msg.textContent = err.message;
    }
}
