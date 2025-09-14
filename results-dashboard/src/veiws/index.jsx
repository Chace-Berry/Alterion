import React, { useEffect, useState } from "react";
import "./dashboard-fixes.css";

const POLL_INTERVAL = 3000;
const JSON_PATH = "/results/lexer-results.json";

function fetchResults(setResults, setError) {
  fetch(JSON_PATH)
    .then((res) => {
      if (!res.ok) {
        throw new Error(`HTTP ${res.status}: ${res.statusText}`);
      }
      return res.json();
    })
    .then((data) => {
      setResults(Array.isArray(data) ? data : []);
      setError(null);
    })
    .catch((error) => {
      console.error("Failed to fetch results:", error);
      setError(error.message);
      setResults([]);
    });
}


function highlightAlterionCode(code) {

  const tokenPatterns = [

    { regex: /(\/\/.*$)/gm, className: "comment", color: "#6a9955" },
    { regex: /(\/\*[\s\S]*?\*\/)/g, className: "comment", color: "#6a9955" },


    { regex: /("(?:[^"\\]|\\.)*")/g, className: "string", color: "#ce9178" },
    { regex: /('(?:[^'\\]|\\.)*')/g, className: "string", color: "#ce9178" },
    { regex: /(`(?:[^`\\]|\\.)*`)/g, className: "string", color: "#ce9178" },


    {
      regex: /\b(\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\b/g,
      className: "number",
      color: "#b5cea8",
    },
    { regex: /\b(0x[0-9a-fA-F]+)\b/g, className: "number", color: "#b5cea8" },


    {
      regex: /\b(component|module|import|export|from|as)\b/g,
      className: "keyword-component",
      color: "#c586c0",
    }, 
    {
      regex: /\b(let|const|var|def|static|final)\b/g,
      className: "keyword-declaration",
      color: "#43d675",
    }, 
    {
      regex:
        /\b(function|fn|method|constructor|class|struct|enum|interface|trait)\b/g,
      className: "keyword-definition",
      color: "#43d675",
    }, 
    {
      regex: /\b(if|else|elif|match|case|switch|when|unless)\b/g,
      className: "keyword-conditional",
      color: "#43d675",
    }, 
    {
      regex: /\b(for|while|loop|do|repeat|until|break|continue)\b/g,
      className: "keyword-loop",
      color: "#43d675",
    }, 
    {
      regex: /\b(return|yield|throw|raise|try|catch|finally|with)\b/g,
      className: "keyword-control",
      color: "#43d675",
    }, 
    {
      regex: /\b(async|await|sync|parallel|concurrent)\b/g,
      className: "keyword-async",
      color: "#43d675",
    }, 
    {
      regex: /\b(public|private|protected|internal|external)\b/g,
      className: "keyword-visibility",
      color: "#43d675",
    }, 
    {
      regex: /\b(true|false|null|undefined|nil|void|this|self|super)\b/g,
      className: "keyword-literal",
      color: "#43d675",
    }, 
    {
      regex: /\b(and|or|not|in|is|typeof|instanceof|new|delete)\b/g,
      className: "keyword-operator",
      color: "#d7ba7d",
    },

    
    {
      regex:
        /\b(String|Int|Float|Double|Bool|Boolean|Array|List|Map|Dict|Set|Object|Any|None|Option|Maybe)\b/g,
      className: "type",
      color: "#4ec9b0",
    },

    
    {
      regex:
        /(\+\+|--|=>|->|<-|\|\||&&|\|=|&=|\^=|<<=|>>=|\*\*=|\+=|-=|\*=|\/=|%=|==|!=|<=|>=|<<|>>|\*\*)/g,
      className: "operator",
      color: "#d4d4d4",
    },
    {
      regex: /([+\-*/%=<>!&|^~?:])/g,
      className: "operator-simple",
      color: "#d4d4d4",
    },

    
    { regex: /([{}[\]()])/g, className: "bracket", color: "#ffd700" },
    { regex: /([;,.])/g, className: "punctuation", color: "#d4d4d4" },

    
    {
      regex: /\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?=\()/g,
      className: "function-call",
      color: "#dcdcaa",
    },

    
    {
      regex: /\.([a-zA-Z_][a-zA-Z0-9_]*)/g,
      className: "property",
      color: "#9cdcfe",
      replacement: '.<span class="property">$1</span>',
    },

    
    {
      regex:
        /\b(?!component|module|import|export|from|as|let|const|var|def|static|final|function|fn|method|constructor|class|struct|enum|interface|trait|if|else|elif|match|case|switch|when|unless|for|while|loop|do|repeat|until|break|continue|return|yield|throw|raise|try|catch|finally|with|async|await|sync|parallel|concurrent|public|private|protected|internal|external|true|false|null|undefined|nil|void|this|self|super|and|or|not|in|is|typeof|instanceof|new|delete|String|Int|Float|Double|Bool|Boolean|Array|List|Map|Dict|Set|Object|Any|None|Option|Maybe)\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g,
      className: "identifier",
      color: "#9cdcfe",
    },
  ];

  let result = code;
  const tokens = [];

  tokenPatterns.forEach((pattern, patternIndex) => {
    let match;
    const regex = new RegExp(pattern.regex);

    while ((match = regex.exec(code)) !== null) {
      tokens.push({
        start: match.index,
        end: match.index + match[0].length,
        text: match[0],
        className: pattern.className,
        color: pattern.color,
        priority: patternIndex,
        replacement: pattern.replacement,
      });
    }
  });

  
  tokens.sort((a, b) => {
    if (a.start !== b.start) return a.start - b.start;
    if (a.end !== b.end) return b.end - a.end; 
    return b.priority - a.priority; 
  });


  const filteredTokens = [];
  for (let i = 0; i < tokens.length; i++) {
    const current = tokens[i];
    const hasOverlap = filteredTokens.some(
      (existing) => current.start < existing.end && current.end > existing.start
    );

    if (!hasOverlap) {
      filteredTokens.push(current);
    }
  }

 
  filteredTokens.sort((a, b) => a.start - b.start);

 
  let highlightedCode = "";
  let lastIndex = 0;

  filteredTokens.forEach((token) => {
    
    highlightedCode += escapeHtml(code.slice(lastIndex, token.start));

   
    if (token.replacement) {
     
      const replacement = token.replacement.replace(
        /\$1/g,
        escapeHtml(token.text.slice(1))
      ); 
      highlightedCode += replacement;
    } else {
      highlightedCode += `<span style="color:${token.color}">${escapeHtml(
        token.text
      )}</span>`;
    }

    lastIndex = token.end;
  });


  highlightedCode += escapeHtml(code.slice(lastIndex));

  return highlightedCode;
}

function escapeHtml(text) {
  const div = document.createElement("div");
  div.textContent = text;
  return div.innerHTML;
}

function renderASTNode(node, depth = 0) {
  if (!node) return null;

  const indent = depth * 20;
  const hasChildren = node.children && node.children.length > 0;

  return (
    <div
      key={`${node.type}-${depth}-${Math.random()}`}
      style={{ marginLeft: `${indent}px`, marginBottom: "4px" }}
    >
      <div
        style={{
          padding: "4px 8px",
          background: node.type === 19 && node.error ? "#ffebee" : "#f5f5f5",
          borderRadius: "4px",
          fontFamily: "monospace",
          fontSize: "0.9rem",
          border:
            node.type === 19 && node.error
              ? "1px solid #e57373"
              : "1px solid #ddd",
        }}
      >
        <strong>Type:</strong> {node.type}
        {node.name && (
          <span>
            {" "}
            | <strong>Name:</strong> "{node.name}"
          </span>
        )}
        {node.value && (
          <span>
            {" "}
            | <strong>Value:</strong> "{node.value}"
          </span>
        )}
        {node.line && (
          <span>
            {" "}
            | <strong>Line:</strong> {node.line}
          </span>
        )}
        {node.column && (
          <span>
            {" "}
            | <strong>Col:</strong> {node.column}
          </span>
        )}
        {node.error && (
          <span style={{ color: "#c62828" }}>
            {" "}
            | <strong>Error:</strong> {node.error}
          </span>
        )}
      </div>
      {hasChildren &&
        node.children.map((child, i) => renderASTNode(child, depth + 1))}
    </div>
  );
}

export default function ResultsDashboard() {

  const [collapsedBlocks, setCollapsedBlocks] = useState({});
  const [results, setResults] = useState([]);
  const [showDiffsOnly, setShowDiffsOnly] = useState(false);
  const [error, setError] = useState(null);
  const [lastUpdated, setLastUpdated] = useState(null);
  const [activeTab, setActiveTab] = useState("tokens");
  const [ast, setAst] = useState(null);
  const [astError, setAstError] = useState(null);
  const [source, setSource] = useState("");
  const [parserProblems, setParserProblems] = useState([]);
  const [problemsOpen, setProblemsOpen] = useState(true);
  const [highlightLine, setHighlightLine] = useState(null);
  const [visibleErrorTooltips, setVisibleErrorTooltips] = useState(new Set());
  const [hoveredLine, setHoveredLine] = useState(null);

  const toggleErrorTooltip = (lineNum) => {
    setVisibleErrorTooltips((prev) => {
      const newSet = new Set(prev);
      if (newSet.has(lineNum)) {
        newSet.delete(lineNum);
      } else {
        newSet.add(lineNum);
      }
      return newSet;
    });
  };


  useEffect(() => {
    if (activeTab === "parser") {
      fetch("/examples/lexer-app-test.alt")
        .then((res) => res.text())
        .then(setSource)
        .catch(() => setSource("// Failed to load source code"));

      fetch("/results/parser-results.json")
        .then((res) => res.json())
        .then((ast) => {
         
          function collectErrors(node, arr = []) {
            if (node && typeof node === "object") {
             
              if (node.type === 19 && node.error) {
                arr.push({
                  message: node.error,
                  line: node.line || 0,
                  column: node.column || 0,
                });
              }

              if (node.error && node.type !== 19) {
                arr.push({
                  message: node.error,
                  line: node.line || 0,
                  column: node.column || 0,
                });
              }

              // Recursively check children
              if (Array.isArray(node.children)) {
                node.children.forEach((child) => collectErrors(child, arr));
              }
            }
            return arr;
          }

          const errors = collectErrors(ast);
          console.log("Found parser errors:", errors); // Debug log
          setParserProblems(errors);
        })
        .catch((err) => {
          console.error("Failed to load parser results:", err);
          setParserProblems([]);
        });
    }
  }, [activeTab, lastUpdated]);

  useEffect(() => {
    if (activeTab === "ast") {
      fetch("/results/ast-results.json")
        .then((res) => {
          if (!res.ok) throw new Error(`HTTP ${res.status}: ${res.statusText}`);
          return res.json();
        })
        .then((data) => {
          setAst(data);
          setAstError(null);
        })
        .catch((err) => {
          setAstError(err.message);
          setAst(null);
        });
    }
  }, [activeTab, lastUpdated]);

  useEffect(() => {
    fetchResults(setResults, setError);
    const interval = setInterval(() => {
      fetchResults(setResults, setError);
      setLastUpdated(new Date());
    }, POLL_INTERVAL);
    return () => clearInterval(interval);
  }, []);

  const filtered = showDiffsOnly
    ? results.filter((r) => r.status === "DIFF")
    : results;

  const diffCount = results.filter((r) => r.status === "DIFF").length;
  const passCount = results.filter((r) => r.status === "OK").length;

  return (
    <div
      style={{
        minHeight: "100vh",
        width: "100vw",
        background: "linear-gradient(135deg, #e0eafc 0%, #cfdef3 100%)",
        display: "flex",
        flexDirection: "column",
        alignItems: "center",
        justifyContent: "flex-start",
        padding: "0",
        margin: "0",
        position: "absolute",
        top: "0",
        left: "0",
        right: "0",
        bottom: "0",
        boxSizing: "border-box",
        fontFamily:
          "-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif",
      }}
    >
      <header
        style={{
          width: "100%",
          background: "#343a40",
          color: "#fff",
          padding: "32px 0 16px 0",
          textAlign: "center",
          boxShadow: "0 2px 8px rgba(0,0,0,0.2)",
        }}
      >
        <h1 style={{ margin: 0, fontSize: "2.5rem", letterSpacing: "2px" }}>
          Alterion Test Results Dashboard
        </h1>
        <p
          style={{ margin: "8px 0 0 0", fontSize: "1.2rem", color: "#b0c4de" }}
        >
          Live parser test results
        </p>
        {lastUpdated && (
          <p
            style={{
              margin: "4px 0 0 0",
              fontSize: "0.9rem",
              color: "#adb5bd",
            }}
          >
            Last updated: {lastUpdated.toLocaleTimeString()}
          </p>
        )}
      </header>

      <main
        style={{
          width: "100%",
          maxWidth: "1200px",
          margin: "32px auto",
          flex: 1,
          display: "flex",
          flexDirection: "column",
          alignItems: "center",
          justifyContent: "flex-start",
          padding: "0 16px",
          boxSizing: "border-box",
        }}
      >
        {/* Tab Controls */}
        <div style={{ display: "flex", gap: "16px", marginBottom: "24px" }}>
          <button
            onClick={() => setActiveTab("tokens")}
            style={{
              padding: "10px 24px",
              fontWeight: "bold",
              fontSize: "1.1rem",
              borderRadius: "8px",
              border: "none",
              background: activeTab === "tokens" ? "#38b2ac" : "#e0eafc",
              color: activeTab === "tokens" ? "#fff" : "#343a40",
              boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
              cursor: "pointer",
            }}
          >
            Tokens
          </button>
          <button
            onClick={() => setActiveTab("ast")}
            style={{
              padding: "10px 24px",
              fontWeight: "bold",
              fontSize: "1.1rem",
              borderRadius: "8px",
              border: "none",
              background: activeTab === "ast" ? "#38b2ac" : "#e0eafc",
              color: activeTab === "ast" ? "#fff" : "#343a40",
              boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
              cursor: "pointer",
            }}
          >
            AST
          </button>
          <button
            onClick={() => setActiveTab("parser")}
            style={{
              padding: "10px 24px",
              fontWeight: "bold",
              fontSize: "1.1rem",
              borderRadius: "8px",
              border: "none",
              background: activeTab === "parser" ? "#38b2ac" : "#e0eafc",
              color: activeTab === "parser" ? "#fff" : "#343a40",
              boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
              cursor: "pointer",
            }}
          >
            Parser
          </button>
        </div>

        {/* Stats and Controls (only show for tokens tab) */}
        {activeTab === "tokens" && (
          <div
            style={{
              display: "flex",
              flexDirection: "row",
              gap: "24px",
              alignItems: "center",
              marginBottom: "24px",
              flexWrap: "wrap",
              justifyContent: "center",
            }}
          >
            <div
              style={{
                background: "#fff",
                padding: "16px 24px",
                borderRadius: "8px",
                boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
                textAlign: "center",
              }}
            >
              <div
                style={{
                  fontSize: "2rem",
                  fontWeight: "bold",
                  color: "#2e7d32",
                }}
              >
                {passCount}
              </div>
              <div style={{ fontSize: "0.9rem", color: "#666" }}>PASS</div>
            </div>

            <div
              style={{
                background: "#fff",
                padding: "16px 24px",
                borderRadius: "8px",
                boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
                textAlign: "center",
              }}
            >
              <div
                style={{
                  fontSize: "2rem",
                  fontWeight: "bold",
                  color: "#c0392b",
                }}
              >
                {diffCount}
              </div>
              <div style={{ fontSize: "0.9rem", color: "#666" }}>DIFF</div>
            </div>

            <button
              onClick={() => setShowDiffsOnly((v) => !v)}
              style={{
                padding: "12px 28px",
                fontWeight: "bold",
                fontSize: "1.1rem",
                borderRadius: "8px",
                border: "none",
                background: showDiffsOnly ? "#ff6b6b" : "#38b2ac",
                color: "#fff",
                boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
                cursor: "pointer",
                transition: "all 0.2s ease",
                transform: "translateY(0)",
              }}
              onMouseOver={(e) =>
                (e.target.style.transform = "translateY(-1px)")
              }
              onMouseOut={(e) => (e.target.style.transform = "translateY(0)")}
            >
              {showDiffsOnly ? "Show All Results" : "Show Only DIFFs"}
            </button>
          </div>
        )}

        {/* Content based on active tab */}
        {activeTab === "tokens" ? (
          // Tokens Table
          filtered.length > 0 ? (
            <div
              style={{
                width: "100%",
                maxWidth: "1000px",
                margin: "0 auto",
                background: "#fff",
                borderRadius: "12px",
                boxShadow: "0 4px 16px rgba(0,0,0,0.1)",
                overflow: "hidden",
              }}
            >
              <div
                style={{
                  background: "#f8f9fa",
                  padding: "24px",
                  borderBottom: "1px solid #e9ecef",
                }}
              >
                <h2
                  style={{
                    margin: 0,
                    fontSize: "1.5rem",
                    color: "#343a40",
                  }}
                >
                  Lexer Results ({filtered.length}{" "}
                  {filtered.length === 1 ? "token" : "tokens"})
                </h2>
              </div>
              <div
                style={{
                  overflowX: "hidden",
                  maxHeight: filtered.length > 8 ? `${8 * 48}px` : "none", // 48px per row
                  minHeight: "0",
                  overflowY: filtered.length > 8 ? "auto" : "visible",
                }}
              >
                <table
                  style={{
                    width: "100%",
                    tableLayout: "fixed",
                    borderCollapse: "collapse",
                    fontSize: "0.95rem",
                  }}
                >
                  <thead>
                    <tr style={{ background: "#f1f3f4" }}>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "7%",
                          wordBreak: "break-word",
                        }}
                      >
                        Index
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "13%",
                          wordBreak: "break-word",
                        }}
                      >
                        Expected Type
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "16%",
                          wordBreak: "break-word",
                        }}
                      >
                        Expected Value
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "13%",
                          wordBreak: "break-word",
                        }}
                      >
                        Returned Type
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "16%",
                          wordBreak: "break-word",
                        }}
                      >
                        Returned Value
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "8%",
                          wordBreak: "break-word",
                        }}
                      >
                        Line
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "8%",
                          wordBreak: "break-word",
                        }}
                      >
                        Column
                      </th>
                      <th
                        style={{
                          padding: "16px 8px",
                          textAlign: "left",
                          fontWeight: "600",
                          fontSize: "1rem",
                          color: "#343a40",
                          borderBottom: "2px solid #e9ecef",
                          width: "11%",
                          wordBreak: "break-word",
                        }}
                      >
                        Status
                      </th>
                    </tr>
                  </thead>
                  <tbody>
                    {filtered.map((r, i) => (
                      <tr
                        key={i}
                        style={{
                          background: i % 2 === 0 ? "#fff" : "#f8f9fa",
                          borderBottom: "1px solid #e9ecef",
                          color: "#111",
                        }}
                      >
                        <td
                          style={{
                            padding: "12px",
                            fontWeight: "500",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.index}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            fontFamily: "monospace",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.expectedType}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            fontFamily: "monospace",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.expectedValue}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            fontFamily: "monospace",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.returnedType}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            fontFamily: "monospace",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.returnedValue}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.line}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          {r.column}
                        </td>
                        <td
                          style={{
                            padding: "12px",
                            fontWeight: "bold",
                            color: r.status === "DIFF" ? "#c0392b" : "#2e7d32",
                            wordBreak: "break-word",
                            textAlign: "left",
                          }}
                        >
                          <span
                            style={{
                              padding: "4px 8px",
                              borderRadius: "4px",
                              fontSize: "0.875rem",
                              background:
                                r.status === "DIFF" ? "#ffcdd2" : "#c8e6c9",
                              color: "#111",
                              wordBreak: "break-word",
                            }}
                          >
                            {r.status}
                          </span>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          ) : (
            <div
              style={{
                marginTop: "64px",
                color: "#888",
                fontSize: "1.2rem",
                textAlign: "center",
                padding: "32px",
                background: "#fff",
                borderRadius: "8px",
                boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
              }}
            >
              {error ? `Failed to load results: ${error}` : "No results found"}
            </div>
          )
        ) : activeTab === "ast" ? (
          // AST View
          <div
            style={{
              width: "100%",
              maxWidth: "900px",
              margin: "0 auto",
              background: "#fff",
              borderRadius: "12px",
              boxShadow: "0 4px 16px rgba(0,0,0,0.1)",
              overflow: "hidden",
              padding: "24px",
            }}
          >
            <h2 style={{ fontSize: "1.5rem", marginBottom: "16px" }}>
              AST (Abstract Syntax Tree)
            </h2>
            {/* Show AST name if available */}
            {ast && ast.name && (
              <div
                style={{
                  fontSize: "1.1rem",
                  color: "#007acc",
                  marginBottom: "12px",
                  fontWeight: "bold",
                }}
              ></div>
            )}
            {astError ? (
              <div
                style={{
                  color: "#c0392b",
                  padding: "16px",
                  background: "#ffebee",
                  borderRadius: "4px",
                }}
              >
                Error loading AST: {astError}
              </div>
            ) : ast ? (
              <div
                style={{
                  maxHeight:
                    Array.isArray(ast.children) && ast.children.length > 10
                      ? `${10 * 48}px`
                      : "none", // 48px per node
                  overflowY:
                    Array.isArray(ast.children) && ast.children.length > 10
                      ? "auto"
                      : "visible",
                  border: "1px solid #ddd",
                  borderRadius: "4px",
                  padding: "16px",
                }}
              >
                {/* Show all AST nodes at root level, enable scroll if more than 10 */}
                {Array.isArray(ast.children)
                  ? ast.children.map((child, i) => renderASTNode(child, 1))
                  : renderASTNode(ast)}
              </div>
            ) : (
              <div style={{ color: "#666", fontStyle: "italic" }}>
                Loading AST...
              </div>
            )}
          </div>
        ) : (

          <div
            style={{
              width: "100%",
              maxWidth: "1100px",
              margin: "0",
              marginLeft: "0",
              alignSelf: "flex-start",
              background: "#1e1e1e",
              borderRadius: "0px",
              boxShadow: "0 4px 20px rgba(0,0,0,0.4)",
              overflow: "hidden",
              border: "1px solid #252526",
              fontFamily: "'Consolas', 'Courier New', monospace",
            }}
          >
            {/* VS Code Title Bar */}
            <div
              style={{
                background: "#2d2d2d",
                height: "35px",
                display: "flex",
                alignItems: "center",
                justifyContent: "space-between",
                borderBottom: "1px solid #252526",
              }}
            >
              <div style={{ display: "flex", alignItems: "center" }}>
                <div style={{ display: "flex", gap: "8px", padding: "0 12px" }}>
                  <div
                    style={{
                      width: "12px",
                      height: "12px",
                      borderRadius: "50%",
                      background: "#ff5f56",
                    }}
                  ></div>
                  <div
                    style={{
                      width: "12px",
                      height: "12px",
                      borderRadius: "50%",
                      background: "#ffbd2e",
                    }}
                  ></div>
                  <div
                    style={{
                      width: "12px",
                      height: "12px",
                      borderRadius: "50%",
                      background: "#27ca3f",
                    }}
                  ></div>
                </div>
                <div
                  style={{
                    color: "#cccccc",
                    fontSize: "13px",
                    marginLeft: "8px",
                  }}
                >
                  app-test.alt — Alterion
                </div>
              </div>
            </div>

            {/* Tab Bar */}
            <div
              style={{
                background: "#252526",
                height: "35px",
                display: "flex",
                alignItems: "center",
                borderBottom: "1px solid #1e1e1e",
              }}
            >
              <div
                style={{
                  background: "#1e1e1e",
                  color: "#ffffff",
                  padding: "8px 16px",
                  fontSize: "13px",
                  borderRight: "1px solid #2d2d30",
                  position: "relative",
                }}
              >
                app-test.alt
                <div
                  style={{
                    position: "absolute",
                    top: "0",
                    left: "0",
                    right: "0",
                    height: "2px",
                    background: "#007acc",
                  }}
                ></div>
              </div>
            </div>

            {/* Main Editor Area */}
            <div style={{ display: "flex", height: "400px" }}>
              {/* Code Editor */}
              <div
                style={{
                  flex: "1",
                  background: "#1e1e1e",
                  position: "relative",
                  overflow: "hidden",
                }}
              >
                {/* Gutter and Code */}
                <div
                  style={{
                    height: "100%",
                    overflowY: "scroll",
                    overflowX: "scroll",
                    fontSize: "14px",
                    lineHeight: "20px",
                    fontFamily: "'Consolas', 'Courier New', monospace",
                    scrollbarWidth: "thin",
                    scrollbarColor: "#424242 #1e1e1e",
                  }}
                  className="vscode-scrollbar"
                >
                  <style jsx>{`
                    .vscode-scrollbar::-webkit-scrollbar {
                      width: 14px;
                      height: 14px;
                    }
                    .vscode-scrollbar::-webkit-scrollbar-track {
                      background: #1e1e1e;
                    }
                    .vscode-scrollbar::-webkit-scrollbar-thumb {
                      background: #424242;
                      border-radius: 0px;
                      border: 3px solid #1e1e1e;
                    }
                    .vscode-scrollbar::-webkit-scrollbar-thumb:hover {
                      background: #4f4f4f;
                    }
                    .vscode-scrollbar::-webkit-scrollbar-corner {
                      background: #1e1e1e;
                    }
                  `}</style>
                  {source.split("\n").map((line, i) => {

                    const openBraces = [];
                    const closeBraces = [];
                    source.split("\n").forEach((l, idx) => {
                      if (l.includes("{")) openBraces.push(idx);
                      if (l.includes("}")) closeBraces.push(idx);
                    });

                    const bracePairs = [];
                    const stack = [];
                    source.split("\n").forEach((l, idx) => {
                      if (l.includes("{")) stack.push(idx);
                      if (l.includes("}")) {
                        if (stack.length) {
                          const start = stack.pop();
                          bracePairs.push({ start, end: idx });
                        }
                      }
                    });

                    const block = bracePairs.find((pair) => pair.start === i);
                    const isCollapsed = block ? collapsedBlocks[i] : false;

                    if (
                      bracePairs.some(
                        (pair) =>
                          pair.start < i &&
                          pair.end >= i &&
                          collapsedBlocks[pair.start]
                      )
                    ) {
                      return null;
                    }
                    const errorOnLine = parserProblems.some(
                      (p) => p.line === i + 1
                    );
                    const errorObj = parserProblems.find(
                      (p) => p.line === i + 1
                    );
                    const lineNum = i + 1;

                    const highlightedLine = highlightAlterionCode(line);

                    return (
                      <div
                        key={i}
                        style={{
                          display: "flex",
                          minHeight: "24px",
                          height: "24px",
                          background:
                            highlightLine === lineNum
                              ? "#2a2d2e"
                              : errorOnLine
                              ? "#3c1e1e"
                              : "transparent",
                          position: "relative",
                          alignItems: "center",
                          padding: "0",
                          margin: "0",
                          boxSizing: "border-box",
                        }}
                        onMouseEnter={() => setHoveredLine(i)}
                        onMouseLeave={() => setHoveredLine(null)}
                      >
                        {/* Collapse arrow - only show on hover if this line has a collapsible block */}
                        <div
                          style={{
                            width: "18px",
                            height: "24px",
                            display: "flex",
                            alignItems: "center",
                            justifyContent: "center",
                            cursor: block ? "pointer" : "default",
                            userSelect: "none",
                            background: "#1e1e1e",
                          }}
                          onClick={
                            block
                              ? () =>
                                  setCollapsedBlocks((prev) => ({
                                    ...prev,
                                    [i]: !prev[i],
                                  }))
                              : undefined
                          }
                        >
                          {block && hoveredLine === i && (
                            <span
                              style={{
                                display: "inline-block",
                                transform: isCollapsed
                                  ? "rotate(-90deg)"
                                  : "rotate(0deg)",
                                transition: "transform 0.2s",
                                fontSize: "12px",
                                color: "#cccccc",
                                lineHeight: "1",
                              }}
                            >
                              <svg
                                xmlns="http://www.w3.org/2000/svg"
                                width="16"
                                height="16"
                                viewBox="0 0 16 16"
                                style={{ display: "block" }}
                              >
                                <path
                                  d="M2 6l6 6 6-6"
                                  fill="none"
                                  stroke="#cccccc"
                                  strokeWidth="2"
                                  strokeLinecap="round"
                                />
                              </svg>
                            </span>
                          )}
                        </div>

                        {/* Line number gutter */}
                        <div
                          style={{
                            width: "60px",
                            height: "24px",
                            background: "#1e1e1e",
                            color: errorOnLine ? "#f14c4c" : "#858585",
                            textAlign: "right",
                            padding: "0 16px 0 0",
                            userSelect: "none",
                            fontSize: "13px",
                            borderRight: "1px solid #252526",
                            flexShrink: 0,
                            lineHeight: "24px",
                            fontWeight: errorOnLine ? "bold" : "normal",
                            display: "flex",
                            alignItems: "center",
                            justifyContent: "flex-end",
                            boxSizing: "border-box",
                          }}
                        >
                          {lineNum}
                        </div>

                        {/* Error indicator */}
                        <div
                          style={{
                            width: "20px",
                            height: "24px",
                            display: "flex",
                            alignItems: "center",
                            justifyContent: "center",
                            background: "#1e1e1e",
                            flexShrink: 0,
                          }}
                        >
                          {errorOnLine && (
                            <div
                              onClick={() => toggleErrorTooltip(lineNum)}
                              style={{
                                width: "10px",
                                height: "10px",
                                borderRadius: "50%",
                                background: "#f14c4c",
                                boxShadow: "0 0 6px #f14c4c88",
                                cursor: "pointer",
                                transition: "transform 0.1s ease",
                                transform: visibleErrorTooltips.has(lineNum)
                                  ? "scale(1.2)"
                                  : "scale(1)",
                              }}
                              onMouseOver={(e) =>
                                (e.target.style.transform = "scale(1.3)")
                              }
                              onMouseOut={(e) =>
                                (e.target.style.transform =
                                  visibleErrorTooltips.has(lineNum)
                                    ? "scale(1.2)"
                                    : "scale(1)")
                              }
                            ></div>
                          )}
                        </div>

                        {/* Code content with enhanced syntax highlighting */}
                        <div
                          style={{
                            flex: "1",
                            color: errorOnLine ? "#f14c4c" : "#d4d4d4",
                            padding: "0 12px",
                            whiteSpace: "pre",
                            position: "relative",
                            lineHeight: "24px",
                            height: "24px",
                            fontWeight: errorOnLine ? "bold" : "normal",
                            letterSpacing: "0.2px",
                            display: "flex",
                            alignItems: "center",
                            overflow: "hidden",
                          }}
                        >
                          <span
                            dangerouslySetInnerHTML={{
                              __html: highlightedLine || " ",
                            }}
                          />
                          {/* Error squiggly underline and error message */}
                          {errorOnLine && (
                            <>
                              <div
                                style={{
                                  position: "absolute",
                                  bottom: "2px",
                                  left: "12px",
                                  right: "12px",
                                  height: "2px",
                                  backgroundImage: `url("data:image/svg+xml,%3csvg width='100' height='100' xmlns='http://www.w3.org/2000/svg'%3e%3cpath d='m0 2l2 -1l1 0l2 1l2 0l1 0l2 -1l2 1l2 0l1 0l2 -1l2 1l2 0l1 0l2 -1l2 1l2 0l1 0l2 -1l2 1l2 0l1 0l2 -1l2 1' stroke='%23f14c4c' fill='none'/%3e%3c/svg%3e")`,
                                  backgroundRepeat: "repeat-x",
                                  backgroundSize: "4px 2px",
                                  opacity: 1,
                                }}
                              ></div>
                              {errorObj &&
                                visibleErrorTooltips.has(lineNum) && (
                                  <div
                                    style={{
                                      position: "absolute",
                                      left: "12px",
                                      top: "-26px",
                                      background: "#1e1e1e",
                                      color: "#f14c4c",
                                      borderRadius: "4px",
                                      padding: "4px 8px",
                                      fontSize: "12px",
                                      fontWeight: "bold",
                                      boxShadow: "0 2px 8px #f14c4c44",
                                      zIndex: 2,
                                      maxWidth: "80vw",
                                      whiteSpace: "pre-wrap",
                                      border: "1px solid #f14c4c",
                                    }}
                                  >
                                    {errorObj.message}
                                  </div>
                                )}
                            </>
                          )}
                        </div>
                      </div>
                    );
                  })}
                </div>
              </div>
            </div>

            {/* Problems Panel */}
            <div
              style={{
                borderTop: "1px solid #252526",
                background: "#1e1e1e",
              }}
            >
              {/* Problems Panel Header */}
              <div
                style={{
                  height: "35px",
                  display: "flex",
                  alignItems: "center",
                  padding: "0 16px",
                  background: "#252526",
                  borderBottom: problemsOpen ? "1px solid #252526" : "none",
                }}
              >
                <button
                  onClick={() => setProblemsOpen((v) => !v)}
                  style={{
                    background: "transparent",
                    border: "none",
                    color: "#cccccc",
                    cursor: "pointer",
                    display: "flex",
                    alignItems: "center",
                    gap: "6px",
                    fontSize: "13px",
                    padding: "4px 0",
                    fontWeight: "normal",
                  }}
                >
                  <span
                    style={{
                      fontSize: "10px",
                      transform: problemsOpen
                        ? "rotate(90deg)"
                        : "rotate(0deg)",
                      transition: "transform 0.2s",
                      transformOrigin: "center",
                    }}
                  >
                    ▶
                  </span>
                  PROBLEMS ({parserProblems.length})
                </button>
              </div>

              {/* Problems List */}
              {problemsOpen && (
                <div
                  style={{
                    maxHeight: "150px",
                    overflowY: "auto",
                    background: "#1e1e1e",
                  }}
                >
                  {parserProblems.length > 0 ? (
                    parserProblems.map((p, i) => (
                      <div
                        key={i}
                        style={{
                          padding: "4px 16px",
                          borderBottom:
                            i < parserProblems.length - 1
                              ? "1px solid #2d2d30"
                              : "none",
                          cursor: "pointer",
                          background:
                            highlightLine === p.line
                              ? "#37373d"
                              : "transparent",
                          display: "flex",
                          alignItems: "center",
                          gap: "8px",
                          fontSize: "13px",
                          color: "#cccccc",
                          minHeight: "22px",
                        }}
                        onClick={() => setHighlightLine(p.line)}
                        onMouseOver={(e) =>
                          (e.currentTarget.style.background = "#37373d")
                        }
                        onMouseOut={(e) =>
                          (e.currentTarget.style.background =
                            highlightLine === p.line
                              ? "#37373d"
                              : "transparent")
                        }
                      >
                        <div
                          style={{
                            width: "16px",
                            height: "16px",
                            borderRadius: "50%",
                            background: "#f14c4c",
                            display: "flex",
                            alignItems: "center",
                            justifyContent: "center",
                            fontSize: "9px",
                            color: "white",
                            fontWeight: "bold",
                            flexShrink: 0,
                          }}
                        >
                          ✕
                        </div>
                        <div
                          style={{
                            flex: "1",
                            overflow: "hidden",
                            textOverflow: "ellipsis",
                            whiteSpace: "nowrap",
                            textAlign: "left",
                          }}
                        >
                          {p.message}
                        </div>
                        <div
                          style={{
                            color: "#858585",
                            fontSize: "12px",
                            flexShrink: 0,
                          }}
                        >
                          app-test.alt:{p.line}:{p.column}
                        </div>
                      </div>
                    ))
                  ) : (
                    <div
                      style={{
                        padding: "12px 16px",
                        color: "#858585",
                        textAlign: "center",
                        fontSize: "13px",
                      }}
                    >
                      No problems found
                    </div>
                  )}
                </div>
              )}
            </div>

            {/* Status Bar */}
            <div
              style={{
                height: "22px",
                background: "#007acc",
                display: "flex",
                alignItems: "center",
                justifyContent: "space-between",
                padding: "0 16px",
                fontSize: "12px",
                color: "#ffffff",
              }}
            >
              <div style={{ display: "flex", gap: "16px" }}>
                <span>Alterion Parser</span>
                <span>{parserProblems.length} problems</span>
              </div>
              <div style={{ display: "flex", gap: "16px" }}>
                <span>Ln {highlightLine || 1}, Col 1</span>
                <span>UTF-8</span>
              </div>
            </div>
          </div>
        )}
      </main>

      <footer
        style={{
          marginTop: "48px",
          textAlign: "center",
          color: "#aaa",
          fontSize: "1rem",
        }}
      >
        &copy; {new Date().getFullYear()} Alterion Test Dashboard
      </footer>
    </div>
  );
}
