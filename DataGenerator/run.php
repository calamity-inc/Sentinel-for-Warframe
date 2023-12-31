<?php
$fh = fopen("../data_codenames.cpp", "w");
fwrite($fh, "#include \"data.hpp\"\r\n\r\n// Generated by DataGenerator\r\n\r\nstd::unordered_map<std::string, std::string> codename_to_english_map = {\r\n");

function addFromExport($name)
{
	global $fh;

	$json = json_decode(file_get_contents("../PublicExport/{$name}_en.json"), true);
	foreach ($json[$name] as $item)
	{
		fwrite($fh, "{\"".$item["uniqueName"]."\", \"".$item["name"]."\"},\r\n");
	}
}

addFromExport("ExportResources");
addFromExport("ExportWarframes");
addFromExport("ExportWeapons");

fwrite($fh, "};\r\n");
fclose($fh);

$fh = fopen("../data_recipes.cpp", "w");
fwrite($fh, "#include \"data.hpp\"\r\n\r\n// Generated by DataGenerator\r\n\r\nstd::unordered_map<std::string, std::string> recipe_to_result_map = {\r\n");

$uniqueIngredients = [];

$json = json_decode(file_get_contents("../PublicExport/ExportRecipes_en.json"), true);
foreach ($json["ExportRecipes"] as $item)
{
	fwrite($fh, "{\"".$item["uniqueName"]."\", \"".$item["resultType"]."\"},\r\n");
	foreach ($item["ingredients"] as $ingredient)
	{
		if (array_key_exists($ingredient["ItemType"], $uniqueIngredients))
		{
			$uniqueIngredients[$ingredient["ItemType"]] = false;
		}
		else
		{
			$uniqueIngredients[$ingredient["ItemType"]] = $item["uniqueName"];
		}
	}
}

fwrite($fh, "};\r\n");
fclose($fh);

$fh = fopen("../data_ingredients.cpp", "w");
fwrite($fh, "#include \"data.hpp\"\r\n\r\n// Generated by DataGenerator\r\n\r\nstd::unordered_map<std::string, std::string> unique_ingredients_to_recipe_map = {\r\n");

foreach ($uniqueIngredients as $ingredient => $recipe)
{
	if ($recipe !== false)
	{
		fwrite($fh, "{\"".$ingredient."\", \"".$recipe."\"},\r\n");
	}
}

fwrite($fh, "};\r\n");
fclose($fh);