Creator "AGD_SERVER"
Version 1.4
AGD_function_list [
	function "de Fraysseix et al."
	function "Schnyder"
	function "Convex"
	function "ConvexDraw"
	function "PlanarStraight"
	function "PlanarDraw"
	function "Mixed-Model"
	function "PureOrthogonal"
	function "QuasiOrthogonalLayout"
	function "OrthogonalLayout"
	function "TreeLayout"
	function "Sugiyama"
	function "Planarization"
	function "Spring Embedder"
	function "Tutte"
	function "Visibility Representation"
]
AGD_option_list [
	function "de Fraysseix et al." [
		work_infos "false"
		infos [
			name: "de Fraysseix, Pach, Pollack"
			author: "H. de Fraysseix, J. Pach, R. Pollack"
			Implementation: "C. Gutwenger"
			date: "february 1997"
			version: "1.0"
		]
	]
	function "Schnyder" [
		work_infos "false"
		infos [
			name: "Schnyder"
			author: "W. Schnyder"
			Implementation: "D. Ambras, D. Alberts"
			date: "February 1997"
			version: ""
		]
	]
	function "Convex" [
		size_opt "true"
		base_ratio 0.330000
		work_infos "false"
		infos [
			name: "Convex (Kant)"
			author: "G. Kant"
			Implementation: "C. Gutwenger"
			date: "March 1997"
			version: "2.0"
		]
	]
	function "ConvexDraw" [
		size_opt "true"
		tside_opt "false"
		base_ratio 0.330000
		work_infos "false"
		infos [
			name: "ConvexDraw (Chrobak/Kant)"
			author: "M. Chrobak, G. Kant"
			Implementation: "C. Gutwenger"
			date: "March 1997"
			version: "2.0"
		]
	]
	function "PlanarStraight" [
		size_opt "true"
		max_outer "true"
		base_ratio 0.330000
		work_infos "false"
		infos [
			name: "PlanarStraight (Gutwenger/Mutzel)"
			author: "C. Gutwenger, P. Mutzel"
			Implementation: "C. Gutwenger"
			date: "March 1997"
			version: "2.0"
		]
	]
	function "PlanarDraw" [
		size_opt "true"
		side_opt "false"
		max_outer "true"
		base_ratio 0.330000
		work_infos "false"
		infos [
			name: "PlanarDraw (Gutwenger/Mutzel)"
			author: "C. Gutwenger, P. Mutzel"
			Implementation: "C. Gutwenger"
			date: "March 1997"
			version: "1.0"
		]
	]
	function "Mixed-Model" [
		base_ratio 0.330000
		work_infos "false"
		infos [
			name: "Mixed-Model (Gutwenger/Mutzel)"
			author: "C. Gutwenger, P. Mutzel"
			Implementation: "C. Gutwenger"
			date: "june 1998"
			version: "2.5"
		]
	]
	function "PureOrthogonal" [
		check "true"
		work_infos "false"
		infos [
			name: "Pure Orthogonal Drawing Algorithm (Tamassia)"
			author: "R.~Tamassia"
			Implementation: "G.~W.~Klau"
			date: "April 1999"
			version: "1.1"
		]
	]
	function "QuasiOrthogonalLayout" [
		check "true"
		work_infos "false"
		infos [
			name: "Quasi-Orthogonal Drawing Algorithm"
			author: "G.~W.~Klau, P.~Mutzel"
			Implementation: "G.~W.~Klau"
			date: "April 1999"
			version: "1.1"
		]
	]
	function "OrthogonalLayout" [
		check "true"
		work_infos "false"
		infos [
			name: "Orthogonal Drawing Algorithm"
			author: "G.~W.~Klau, P.~Mutzel"
			Implementation: "G.~W.~Klau"
			date: "April 1999"
			version: "1.1"
		]
	]
	function "TreeLayout" [
		sib_separation 4.00000
		subtree_separation 4.00000
		level_separation 4.00000
		left_margin 0.000000
		upper_margin 0.000000
		work_infos "false"
		infos [
			name: "Layout algorithm for general trees"
			author: "E. Reingold, J. Tilford, J. Q. Walker II"
			Implementation: "S. Leipert"
			date: "May 1997"
			version: "0.1"
		]
	]
	function "Sugiyama" [
		ranking [
			ranking_wahl 0
			ranking_lst [
				lst "LongestPath"
				lst "DFS"
			]
		]
		heuristic [
			heuristic_wahl 0
			heuristic_lst [
				lst "Split"
				lst "Barycenter"
				lst "Median"
				lst "Greedy Insert"
				lst "Greedy Switch"
			]
		]
		fixed_level_distance "false"
		node_distance 3.00000
		level_distance 3.00000
		work_infos "false"
		infos [
			name: "Sugiyama"
			author: "K. Sugiyama, S. Tagawa, M. Toda"
			Implementation: "C. Gutwenger, M. KrÅger"
			date: "March 1998"
			version: "2.0"
		]
	]
	function "Planarization" [
		layouter [
			layouter_wahl 8
			layouter_lst [
				lst "de Fraysseix et al."
				lst "Schnyder"
				lst "Convex"
				lst "ConvexDraw"
				lst "PlanarStraight"
				lst "PlanarDraw"
				lst "Mixed-Model"
				lst "PureOrthogonal"
				lst "QuasiOrthogonalLayout"
				lst "OrthogonalLayout"
			]
		]
		work_infos "false"
		infos [
			name: "Planarization"
			author: ""
			Implementation: "C. Gutwenger"
			date: "decembre 1997"
			version: "1.0"
		]
	]
	function "Spring Embedder" [
		noise "true"
		iterations 400
		work_infos "false"
		infos [
			name: "Spring Embedder (Fruchterman/Reingold)"
			author: "T. Fruchterman, E. Reingold"
			Implementation: "S. NÑher, D. Alberts"
			date: "April 1997"
			version: ""
		]
	]
	function "Tutte" [
		work_infos "false"
		infos [
			name: "Tutte"
			author: "W. T. Tutte"
			Implementation: "D. Alberts"
			date: "April 1997"
			version: ""
		]
	]
	function "Visibility Representation" [
		work_infos "false"
		infos [
			name: "Weak Visibility Representation"
			author: "P. Rosenstiehl, R. E. Tarjan"
			Implementation: "D. Alberts"
			date: "1996/1997"
			version: ""
		]
	]
]

Creator "AGD_SERVER"
Version 1.4

Creator "AGD_SERVER"
Version 1.4

Creator "AGD_SERVER"
Version 1.4

Creator "AGD_SERVER"
Version 1.4
