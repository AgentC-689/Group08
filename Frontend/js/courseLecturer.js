// Part 3 (Member 3): Course Catalog and Lecturer Assigned Classes UI logic (FR06, FR08)
async function loadCourses() {
    const courses = await apiRequest("/courses");
    document.getElementById("coursesBody").innerHTML = courses.map(c => `
        <tr>
            <td>${c.course_code}</td>
            <td>${c.course_name}</td>
            <td>${c.credits}</td>
        </tr>
    `).join("");
}

async function createCourse(event) {
    event.preventDefault();
    await apiRequest("/courses", {
        method: "POST",
        body: JSON.stringify({
            course_code: document.getElementById("crsCode").value.trim(),
            course_name: document.getElementById("crsName").value.trim(),
            credits: Number(document.getElementById("crsCredits").value)
        })
    });
    event.target.reset();
    loadCourses();
}

async function loadMyClasses() {
    const classes = await apiRequest("/lecturers/my-classes");
    document.getElementById("myClassesBody").innerHTML = classes.map(c => `
        <tr>
            <td>${c.class_code}</td>
            <td>${c.class_name}</td>
            <td>${c.course_name}</td>
            <td>${c.schedule}</td>
            <td>${c.room}</td>
            <td><a href="/lecturer_attendance.html" class="btn">Open Attendance</a></td>
        </tr>
    `).join("");
}
