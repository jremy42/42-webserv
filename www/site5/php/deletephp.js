let deleteButton = document.getElementById("deleteButton");
deleteButton.addEventListener("click", function() {

	fetch("delete.php", {
		method: "DELETE",
	}).then(function(response) {
		if (response.status == 200)
		{
			alert("all Files deleted");
		}
		else
		{
			alert("Dir not found");
		}
	});
});