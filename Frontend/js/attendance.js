// Part 4 (Member 4): Session Creation, Manual Attendance, GPS Self Check-In & Audit Edit (FR09, FR10, FR15, FR16)
async function loadLecturerSessions() {
    const sessions = await apiRequest("/sessions");
    const sel = document.getElementById("sessionSelect");
    if (!sel) return;
    sel.innerHTML = sessions.map(s =>
        `<option value="${s.session_id}">#${s.session_id} | ${s.class_code} | ${s.session_date} (${s.start_time}-${s.end_time}) | Code: ${s.session_code} [${s.session_status}]</option>`
    ).join("");
    if (sessions.length) loadSessionRoster();
}

async function createSession(event) {
    event.preventDefault();
    try {
        const res = await apiRequest("/sessions", {
            method: "POST",
            body: JSON.stringify({
                class_id: Number(document.getElementById("sessClassId").value),
                session_date: document.getElementById("sessDate").value,
                time_slot: document.getElementById("sessSlot").value.trim(),
                start_time: document.getElementById("sessStart").value,
                end_time: document.getElementById("sessEnd").value,
                require_location: document.getElementById("sessRequireGps").checked,
                class_latitude: Number(document.getElementById("sessLat").value || 0),
                class_longitude: Number(document.getElementById("sessLng").value || 0),
                allowed_distance: Number(document.getElementById("sessDist").value || 100)
            })
        });
        alert(`Session #${res.session_id} created! Share Session Code: ${res.session_code}`);
        loadLecturerSessions();
    } catch (err) {
        alert(err.message);
    }
}

async function loadSessionRoster() {
    const sessionId = document.getElementById("sessionSelect").value;
    if (!sessionId) return;
    const roster = await apiRequest(`/sessions/${sessionId}/roster`);
    document.getElementById("rosterBody").innerHTML = roster.map(r => `
        <tr class="${r.is_outside_area ? 'row-alert' : ''}">
            <td>${r.student_code}</td>
            <td>${r.full_name}</td>
            <td>${r.is_outside_area ? '<span class="badge badge-warning">Outside class area</span>' : 'Valid'}</td>
            <td><strong>${r.status}</strong>${r.old_status ? ` <small>(was: ${r.old_status})</small>` : ''}</td>
            <td>${r.edit_reason || '-'}</td>
            <td>
                ${["Present", "Absent", "Late", "Excused"].map(st =>
                    `<button class="btn btn-secondary" style="padding:4px 8px;margin-right:4px;font-size:12px;"
                             onclick="markOrEdit(${sessionId}, ${r.student_id}, ${r.record_id}, '${st}')">${st}</button>`
                ).join("")}
            </td>
        </tr>
    `).join("");
}

async function markOrEdit(sessionId, studentId, recordId, status) {
    if (recordId > 0) {
        const reason = prompt("Enter reason for editing this attendance record (required by FR10):");
        if (!reason) return;
        await apiRequest(`/attendance/records/${recordId}`, {
            method: "PUT",
            body: JSON.stringify({ status, edit_reason: reason })
        });
    } else {
        await apiRequest(`/sessions/${sessionId}/attendance`, {
            method: "POST",
            body: JSON.stringify({ student_id: studentId, status })
        });
    }
    loadSessionRoster();
}

async function closeSelectedSession() {
    const sessionId = document.getElementById("sessionSelect").value;
    if (!sessionId) return;
    const res = await apiRequest(`/sessions/${sessionId}/close`, { method: "POST" });
    alert(`Session closed. Unchecked students marked Absent: ${res.auto_marked_absent}`);
    loadLecturerSessions();
}

async function loadOpenStudentSessions() {
    const sessions = await apiRequest("/sessions");
    const sel = document.getElementById("studentSessionSelect");
    if (!sel) return;
    sel.innerHTML = sessions.filter(s => s.session_status === "Open").map(s =>
        `<option value="${s.session_id}">Session #${s.session_id} - ${s.class_code} (${s.session_date} ${s.start_time}-${s.end_time})</option>`
    ).join("") || `<option value="">No open sessions available</option>`;
}

function submitStudentCheckIn(event) {
    event.preventDefault();
    const sessionId = Number(document.getElementById("studentSessionSelect").value);
    const code = document.getElementById("checkInCode").value.trim().toUpperCase();
    const statusBox = document.getElementById("checkInStatus");

    const sendRequest = async (lat, lng) => {
        try {
            const payload = { session_id: sessionId, session_code: code };
            if (lat !== null && lng !== null) {
                payload.device_latitude = lat;
                payload.device_longitude = lng;
            }
            const res = await apiRequest("/attendance/check-in", { method: "POST", body: JSON.stringify(payload) });
            statusBox.style.display = "block";
            statusBox.textContent = res.message;
        } catch (err) {
            statusBox.style.display = "block";
            statusBox.textContent = err.message;
        }
    };

    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
            pos => sendRequest(pos.coords.latitude, pos.coords.longitude),
            () => sendRequest(null, null)
        );
    } else {
        sendRequest(null, null);
    }
}
