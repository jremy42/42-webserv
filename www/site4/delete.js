let fileToDelete = document.getElementById("fileToDelete");
let deleteButton = document.getElementById("deleteButton");
deleteButton.addEventListener("click", function() {
	if (fileToDelete.value == "")
	{
		alert("Please enter a file name");
		return;
	}
	fetch("/upload/" + fileToDelete.value, {
		method: "DELETE",
	}).then((e) => {
		if (e.status == 200)
			alert("File deleted successfully");
		else
			alert("Error deleting file");
	});
});