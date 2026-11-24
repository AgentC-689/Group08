// Part 2 (Member 2): Student Management, CSV Import, and Class Scheduling UI logic (FR04, FR05, FR07)
async function loadStudents() {
    const q = document.getElementById("searchQuery") ? document.getElementById("searchQuery").value.trim() : "";
    const students = await apiRequest("/students?q=" + encodeURIComponent(q));
    document.getElementById("studentsBody").innerHTML = students.map(s => `
        <tr>
            <td>${s.student_code}</td>
            <td>${s.full_name}</td>
            <td>${s.email}</td>
            <td>${s.class_code}</td>
            <td><button class="btn btn-danger" onclick="removeStudent(${s.student_id})">Delete</button></td>
        </tr>
    `).join("");
}

async function createStudent(event) {
    event.preventDefault();
    await apiRequest("/students", {
        method: "POST",
        body: JSON.stringify({
            student_code: document.getElementById("stCode").value.trim(),
            full_name: document.getElementById("stName").value.trim(),
            email: document.getElementById("stEmail").value.trim(),
            class_id: Number(document.getElementById("stClassId").value)
        })
    });
    event.target.reset();
    loadStudents();
}

async function removeStudent(id) {
    if (!confirm("Delete this student?")) return;
    await apiRequest(`/students/${id}`, { method: "DELETE" });
    loadStudents();
}

async function importCsvStudents() {
    const fileInput = document.getElementById("csvFile");
    if (!fileInput.files.length) return alert("Please select a CSV file.");
    const text = await fileInput.files[0].text();
    const res = await apiRequest("/students/import", {
        method: "POST",
        headers: { "Content-Type": "text/csv" },
        body: text
    });
    alert(`Imported ${res.imported} students successfully.`);
    loadStudents();
}

async function loadClasses() {
    const classes = await apiRequest("/classes");
    document.getElementById("classesBody").innerHTML = classes.map(c => `
        <tr>
            <td>${c.class_code}</td>
            <td>${c.class_name}</td>
            <td>${c.course_name}</td>
            <td>${c.lecturer_name}</td>
            <td>${c.schedule}</td>
            <td>${c.room}</td>
        </tr>
    `).join("");
}

async function createClass(event) {
    event.preventDefault();
    try {
        await apiRequest("/classes", {
            method: "POST",
            body: JSON.stringify({
                class_code: document.getElementById("clsCode").value.trim(),
                class_name: document.getElementById("clsName").value.trim(),
                course_id: Number(document.getElementById("clsCourseId").value),
                lecturer_id: Number(document.getElementById("clsLecturerId").value),
                schedule: document.getElementById("clsSchedule").value.trim(),
                room: document.getElementById("clsRoom").value.trim()
            })
        });
        event.target.reset();
        loadClasses();
    } catch (err) {
        alert(err.message); // Displays schedule/room conflict warning (UC04 2a)
    }
}
