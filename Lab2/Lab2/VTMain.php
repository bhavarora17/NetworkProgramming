<!DOCTYPE html>
<html>
<head>

	<?php

	$namedno = getenv('EPPN');
	$name = explode("@", $namedno);
	$present = "false";
	$presentIns = "false";
		
	$dbhost = 'localhost';
	$dbuser = 'perdba';
	$dbpass = 'BcmJXfJOh6m';
	$conn = mysql_connect($dbhost, $dbuser, $dbpass);
	if(! $conn )
	{
  		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('OSU_PER');
	
	$sql = "SELECT * FROM instadmin WHERE userid='$name[0]' and Type='admin'";
	$res = mysql_query($sql);
	$count = mysql_num_rows($res);
	if($count == 1) { $present = "true"; }

	$sql1 = "SELECT * FROM instadmin WHERE userid='$name[0]' and Type='instructor'";
	$res1 = mysql_query($sql1);
	$count1 = mysql_num_rows($res1);
	if($count1 == 1) { $presentIns = "true"; }

	mysql_close($conn);
	?>

	<!-- META -->
	<title>Vector Practice Module</title>
	<meta charset="UTF-8">

	<!--javascript-->
	<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js"></script>
	<script type="text/javascript">

	var name,nameno = [],rolelst,role = [],present,presentIns;
	
	$(function () 
  	{
       	name = '<?php echo getenv('EPPN');?>';
		nameno = name.split("@");
		document.getElementById("uidInput").value = nameno[0];  
		rolelst = '<?php echo getenv('AFFILIATION');?>';
		role = rolelst.split("@");
    	});

	function GoToStHome()
	{
		if(document.getElementById("inst").checked)
		{
			presentIns = '<?php echo $presentIns;?>';
			if(presentIns == "true")
				location.href = "InstructorHome.php?name="+nameno[0];
			else
				alert("You are not Authorized to view this content");
		}
		else if(document.getElementById("admin").checked)
		{
			present = '<?php echo $present;?>';
			if(present == "true")
				location.href = "AdminHome.php?name="+nameno[0];
			else
				alert("You are not Authorized to view this content");
		}
		else
			giveAttendance();
	}

	function giveAttendance()
	{
		$.ajax({
		    type : "POST",
		    url : "php/attendance.php",
		    dataType : 'json', 
		    success: function(html) {
 			if(html.returned_val == "Success")
				location.href = "VTHome.php?name="+nameno[0];
      			}
		});
	}
	//
	
	</script>
	
	<!-- CSS -->
	<link rel="stylesheet" type="text/css" href="css/kickstart.css" media="all" />
</head>
<body>
<div class="grid">
	<div class="col_12" style="margin-top:50px;">
		<h4 class="center">Vector Practice Module</h4>
		<div style="margin-left:95%;">	<button onclick="open_window()">Troubleshooting</a></div>
		
		<h6 class="center">
		<i class="icon-fire"></i></br>
		Points To Your Future Success</h6>
		</br></br></br></br></br>
	</div>
	<div class="center" style="display: block">
	<div id="form-example" class="tab-content">
	
	<h6 class = "center">Welcome</h6>
	<input name="uidInput" type="text" id="uidInput"></br></br>

	<input id = "student" type="radio" name="choice" value="student" checked = "checked">Student&nbsp;&nbsp;&nbsp;
	<input id = "inst" type="radio" name="choice" value="instructor">Instructor&nbsp;&nbsp;&nbsp;
	<input id = "admin" type="radio" name="choice" value="admin">Administrator
	</br></br>	
	
	<button type="submit " onclick= "GoToStHome();">GO</button></br></br></br>
	
	
	<script>

	//code to focus the cursor on the textbox on loading of page
	//and also for the Enter key to submit
	//notice that the button type above is submit
	function open_window() {
		var myWindow = window.open("troubleshoot.php", "MsgWindow", "width=1000, height=500");
		}

	
	
	
    	$('#uidInput').focus();
    document.getElementById('uidInput').addEventListener('keypress', function(event){if (event.keyCode == 13){GoToStHome();}});
    document.getElementById('student').addEventListener('keypress', function(event) {if (event.keyCode == 13){GoToStHome();}});
    document.getElementById('admin').addEventListener('keypress', function(event) {if (event.keyCode == 13){GoToStHome();}});
    document.getElementById('inst').addEventListener('keypress', function(event) {if (event.keyCode == 13){GoToStHome();}});

</script>	



	</div>
	</div>
</div>
 <!-- End Grid -->
</body>
</html>

