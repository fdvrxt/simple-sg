# simple-sg

**simple-sg** is a static site generator written in C++. It converts Markdown files with JSON frontmatter into HTML using [md4c](https://github.com/mity/md4c) for Markdown parsing and [Inja](https://github.com/pantor/inja) for templating. Sites are themed, include automatic tag/index support, and can be served locally with live reload while you edit content.

## Prerequisites

- A C++17-compatible compiler
- [CMake](https://cmake.org/) 3.12 or later
- [md4c](https://github.com/mity/md4c) static libraries (`md4c.lib`, `md4c-html.lib`) placed in `includes/`
- [Inja](https://github.com/pantor/inja) and [nlohmann/json](https://github.com/nlohmann/json) headers available in `includes/`
- Python 3 (only required for the live-reload server mode)

## Building

```bash
cmake -S . -B build
cmake --build build
```

## Site structure

Create a site directory with the following layout:

```
site/
├── assets/                # Optional static assets for your site
├── content/               # Markdown content with JSON frontmatter
│   └── posts/
│       └── my-first-post.md
├── output/                # Generated HTML is written here
├── themes/
│   └── simple-blog/       # Example theme (see https://github.com/fdvrxt/simple-blog)
└── config.json            # Site configuration
```

### Frontmatter format

Markdown files must start with JSON frontmatter delimited by `---` lines. The JSON describes how the page is rendered and any additional metadata:

```md
---
{
  "template": "post",
  "title": "My First Post",
  "date": "2025-03-11",
  "tags": ["Announcements", "Welcome"]
}
---
Your markdown content goes here.
```

Each page must specify a `template` defined by the active theme. Tags may be supplied as a string or array; they are normalized and used to build site-wide tag lists.

### config.json

Example configuration using the `simple-blog` theme.

```json
{
	"url": "http://localhost:5500",
	"title": "My First Blog",
	"theme": "simple-blog",
	"description": "My first blog!",
	"params": {
	    "menus": [
		{
			"name": "Home",
			"url": "/"
		},
		{
			"name": "Tags",
			"url": "/tags"
		},
		{
			"name": "Contact",
			"url": "/contact.html"
		},
		{
			"name": "About",
			"url": "/about.html"
		}
		],
		"favicon_dir": "/assets/favicon"
	}
}
```

- `url` (required): Base URL used to build canonical links.
- `title`: Falls back to `Site` if omitted.
- `description`: Defaults to `Very cool website.` if omitted.
- `theme` (required): Name of the theme folder inside `themes/`.
- `params`: Arbitrary values passed through to the theme templates.

Themes include their own `config.json` (e.g., mapping template names and assets directory). Any `directives` declared there can enable features such as site indexes or tag pages.

## Usage

From the root of your site directory (where `config.json` lives), run the generator:

```bash
/path/to/simple-sg [config.json]
```

- Omitting the argument defaults to `./config.json` in the current directory.
- The resulting HTML can be found in `output/`.

### Live-reload server

Serve the generated site with automatic rebuilds and browser reloads:

```bash
/path/to/simple-sg server [config.json]
```

- Watches the `content/` directory for changes and rebuilds when files change.
- Starts `python -m http.server` (auto-detected Python 3 command) and injects a live-reload snippet into rendered pages.
- Press `Ctrl+C` to stop the server.

## Theme directives

Themes can declare build-time directives in their `config.json` (under `theme.directives`). Directives include:

- `index` – marks that an index page should be generated with available posts.
- `tags` – collects tags from page frontmatter, generates tag metadata, and makes it available to templates.
