const http = require("http")
const path = require("path")
const fs = require("fs")

class FileServer {
	static NOTFOUND_HTML = `
    <html>
        <body>
            <h1>404 NOT FOUND</h1>
        </body>
    </html>`

	static EXTNAME_TO_CONTENT_TYPE = new Map([
		[".wasm", "application/wasm"],
		[".js", "application/javascript"],
		[".html", "text/html"],
	])
	constructor(rootDir, port) {
		this.rootDir = rootDir
		this.port = port
		this.server = http.createServer(this.listener.bind(this))
		this.server.listen(port)
	}

	listener(req, res) {
		const fullPath = path.join(this.rootDir, req.url)

		if (!fs.existsSync(fullPath)) {
			res.setHeader("Content-Type", "text/html")
			res.writeHead(400, "Not Found")
			res.write(FileServer.NOTFOUND_HTML)
			res.end()
			return
		}

		const fileStat = fs.statSync(fullPath)

		if (fileStat.isFile()) {
			const ext = path.extname(fullPath)
			const contentType = FileServer.EXTNAME_TO_CONTENT_TYPE.get(ext)

			res.setHeader("Cross-Origin-Opener-Policy", "same-origin")
			res.setHeader("Cross-Origin-Embedder-Policy", "require-corp")
			res.setHeader("Content-Type", contentType ?? "text/plain")

			fs.createReadStream(fullPath).pipe(res)
		} else if (fileStat.isDirectory()) {
			const dir = fs.opendirSync(fullPath)
			const dirContents = []
			let dirContent
			while (((dirContent = dir.readSync()), dirContent)) {
				dirContents.push(`${dirContent.name}`)
			}
			dir.close()

			res.setHeader("Content-Type", "text/html")
			res.write(`
            <html>
                <body>
                    <h1>${path.basename(fullPath)}</h1>
                    <ul>
                    ${dirContents
											.map((name) => `<li> <a href="${path.join(req.url, name)}">${name}</a></li>`)
											.join("\n")}
                    </ul>
                </body>
            </html>            

            `)
			res.end()
		}
	}
}

const port = process.argv[2]
const dirPath = process.argv[3]

new FileServer(dirPath, port)
