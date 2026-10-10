#import "@preview/basic-resume:0.2.9": *

// Personal information
#let name = "Joel Joseph Mathews"
#let location = "Kottayam, Kerala"
#let email = "joeljosephcl10@gmail.com"
#let github = "github.com/Joeljm1"
#let phone = "+91 8891603761"

#show: resume.with(
  author: name,
  location: location,
  email: email,
  github: github,
  phone: phone,
  accent-color: "#26428b",
  font: "New Computer Modern",
  paper: "us-letter",
  author-position: left,
  personal-info-position: left,
)

== Education

#edu(
  institution: "Indian Institute of Information Technology, Kottayam (4th Year)",
  location: "Valavoor, Palai, Kottayam",
  dates: dates-helper(start-date: "Aug 2023", end-date: "Present"),
  degree: "B.Tech in Computer Science and Engineering",
)

- Cumulative GPA: 9.51
- Relevant Coursework: Operating Systems, Computer Networks, Database Management, Compiler Design, DSA, Computer Organization & Architecture, Artificial Intelligence, Cloud Computing.

== Open Source

#project(
  name: "LLVM Compiler Infrastructure (llvm/llvm-project)",
  url: "https://github.com/llvm/llvm-project/pull/183772",
  urlName: "Pull Request",
)

- Extended SelectionDAG backend analysis to support vector `ISD::SHL` operations with `DemandedElts` tracking and added upstream regression test suites.

== Experience

#work(
  company: "Inktek Solutions",
  title: "React Native Developer Intern",
  dates: dates-helper(start-date: "June 2026", end-date: "September 2026"),
)

- Developed a full-featured cross-platform mobile application from scratch using React Native, owning the client-side architecture and lifecycle.
- Integrated RESTful backend APIs for secure user authentication, state synchronization, and real-time data handling.
- Conducted unit and functional end-to-end testing to ensure cross-device stability and smooth UI performance.

== Projects

#project(
  name: "Game scouter",
  url: "https://github.com/Joeljm1/game-scouter",
  urlName: "Github",
)

- Developed a minimal dependency REST API in Go leveraging standard library packages for a minimal binary footprint and high-throughput performance.
- Engineered a stateful session management system featuring a custom in-memory LRU cache and write-on-change persistence, benchmarked to eliminate per-request database overhead.
- Engineered OWASP compliant security infrastructure from scratch (no third-party libraries), including a token bucket rate limiter for DoS mitigation, custom CSRF/CORS middleware, and an OIDC authentication flow with nonce-based replay protection.
- Integrated full-stack observability using OpenTelemetry (OTel) to stream distributed traces to Jaeger, structured logs to Loki, and metrics to Prometheus & Grafana.
- Leveraged Go concurrency primitives (goroutines and channels) to execute asynchronous background jobs, cleanup tasks, and request orchestration off critical request paths.

#project(
  name: "Epoll Chat",
  url: "https://github.com/Joeljm1/epollchat",
  urlName: "Github",
)

- Developed a concurrenct, single-threaded TCP chat server in C++23 leveraging Linux `epoll` for event-driven I/O multiplexing.
- Engineered a non-blocking event loop using level-triggered epoll , multiplexing concurrent client connections with minimal memory footprint and zero thread-switching overhead.
- Implemented per client non blocking output queues to handle socket backpressure and partial writes and prevent event loop blocking.
- Designed a newline-delimited message-framing protocol with per-client line buffering and room based broadcasting, and handled clean socket teardown on disconnect.

== Skills

- *Languages*: Go, C++, C, Python, TypeScript, JavaScript, Lua, Bash, SQL, HTML/CSS
- *Frameworks & Libraries*: React, React Native, Hono.js, Bubble Tea, Raylib, Astro, SvelteKit
- *Cloud, Databases & Systems*: Cloudflare Workers & Durable Objects, GCP, Docker, PostgreSQL, SQLite (D1), Linux, gRPC
- *Tools & Observability*: Git, Make, GDB, Neovim, OpenTelemetry (OTel), Prometheus, Grafana, Curl
