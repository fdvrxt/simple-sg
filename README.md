# simple-sg
**simple-sg** is a simple static site generator written in C++. It's currently a WIP but is usable in its current state.

## Compilation

To compile **simple-sg**, you'll need to:

1. **Compile and link against** [md4c](https://github.com/mity/md4c).
2. **Include the following heades**
   - [Inja](https://github.com/pantor/inja)
   - [json.hpp](https://github.com/nlohmann/json)
3. **Use a C++17-compatible compiler**.


## Example Usage

1. **Download a theme**  
   The only available theme for simple-sg is [simple-blog](https://github.com/fdvrxt/simple-blog). However, it shouldn't be difficult to modify other 
   themes that use the Inja rendering engine to work with **simple-sg**.

2. **Set up your project directory**  
	Create the following folder structure:

   ```
   /blog-instance
   ├── /content
   │   ├── /posts
   │   │   └── my-first-post.md
   │   └── index.md
   ├── /output
   ├── /resources
   ├── /themes
   │   └── /simple-blog
   └── config.json
   ```

   - Markdown files inside `/content` will be converted to HTML files with the same filename.
   - Each file must specify a template from the theme in its JSON frontmatter.
   - Frontmatter is enclosed between `---` lines at the beginning of the file.

3. **Define Frontmatter for Pages**  
   The **simple-blog** theme provides two templates:
   - **`index`** Renders the blog's post feed.
   - **`post`** Renders a single blog post.

   Example **index.md**:
   
   ```md
   ---
   {
       "template": "index"
   }
   ---
   ```

   Example **`my-first-post.md`**:
   
   ```md
   ---
   {
       "title": "My First Post",
       "date": "11-03-2025",
       "template": "post"
   }
   ---
   My first blog post!
   ```

4. **Edit config.json**:
    Specify the url, title, and theme of the blog. `params` are optional parameters that can
    be passed to the theme. In this example they control the menus in the header of the blog. 
    ```
    {
    	"url"			: "http://localhost:5500",
    	"title"			: "My First Blog",
        "theme"			: "simple-blog",
    	"params"		: 
    	{
    		"menus": 
    		{
    			"Home"		: "/",
    			"About"		: "/about.html",
    			"Contact"	: "/contact.html"
    		}
    	}
    }
    ```
4. **Run simple-sg**  
   Navigate to the root of your project (`/blog-instance`) and run:

   ```bash
   ./simple-sg
   ```

   Alternatively, you can specify the full path to `config.json` as an argument:

   ```bash
   ./simple-sg /path/to/blog-instance/config.json
   ```

5. **View the Output**  
   The generated HTML files will be placed in the `/output` directory.
