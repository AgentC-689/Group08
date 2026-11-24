// Part 5 (Member 5): Dashboard KPIs, History Filters, Report Statistics, >20% Warning & CSV Export (FR11-FR14)
async function loadDashboard() {
    const kpi = await apiRequest("/dashboard/summary");
    document.getElementById("kpiStudents").textContent = kpi.total_students;
    document.getElementById("kpiClasses").textContent = kpi.total_classes;
    document.getElementById("kpiCourses").textContent = kpi.total_courses;
    document.getElementById("kpiRate").textContent = kpi.attendance_rate + "%";

    const history = await apiRequest("/history");
    document.getElementById("recentBody").innerHTML = history.slice(0, 10).map(r => `
        <tr>
            <td>${r.student_code}</td>
            <td>${r.student_name}</td>
            <td>${r.class_code}</td>
            <td><span class="badge ${r.status === 'Present' ? 'badge-present' : 'badge-absent'}">${r.status}</span></td>
        </tr>
    `).join("");
}

async function loadHistory() {
    const status = document.getElementById("filterStatus").value;
    const date = document.getElementById("filterDate").value;
    const rows = await apiRequest(`/history?status=${encodeURIComponent(status)}&date=${encodeURIComponent(date)}`);
    document.getElementById("historyBody").innerHTML = rows.length ? rows.map(r => `
        <tr>
            <td>${r.session_date}</td>
            <td>${r.class_code}</td>
            <td>${r.student_code}</td>
            <td>${r.student_name}</td>
            <td>${r.status}</td>
            <td>${r.is_outside_area ? "Outside class area" : "Valid"}</td>
            <td>${r.edit_reason || "-"}</td>
        </tr>
    `).join("") : `<tr><td colspan="7">No data</td></tr>`;
}

async function loadReport() {
    const classId = document.getElementById("reportClassId").value;
    const data = await apiRequest("/reports?class_id=" + encodeURIComponent(classId));
    document.getElementById("overallSummary").textContent =
        `Present: ${data.overall_present_percent}% | Absent: ${data.overall_absent_percent}%`;

    document.getElementById("reportBody").innerHTML = data.students.map(s => `
        <tr class="${s.absence_warning ? 'row-alert' : ''}">
            <td>${s.student_code}</td>
            <td>${s.full_name}</td>
            <td>${s.total_sessions}</td>
            <td>${s.present}</td>
            <td>${s.absent}</td>
            <td>${s.late}</td>
            <td>${s.attendance_rate}%</td>
            <td>${s.absence_warning ? '<span class="badge badge-warning">Warning (>20% Absent)</span>' : 'Normal'}</td>
        </tr>
    `).join("");
}

async function exportCsvReport() {
    const csvText = await apiRequest("/reports/export");
    const blob = new Blob([csvText], { type: "text/csv;charset=utf-8;" });
    const link = document.createElement("a");
    link.href = URL.createObjectURL(blob);
    link.download = "attendance_report.csv";
    link.click();
}
