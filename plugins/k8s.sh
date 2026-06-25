#!/usr/bin/env bash
# Kubernetes current context and namespace plugin for tinyfetch
set -euo pipefail

# Check if kubectl exists
if ! command -v kubectl >/dev/null 2>&1; then
  exit 0
fi

# Check if kubeconfig file exists in default locations
if [ ! -f "${KUBECONFIG:-$HOME/.kube/config}" ] && [ ! -d "$HOME/.kube" ]; then
  exit 0
fi

# Get current context
context=$(kubectl config current-context 2>/dev/null || echo "")

# Exit if no context active
if [ -z "$context" ]; then
  exit 0
fi

# Get current namespace
namespace=$(kubectl config view --minify --output 'jsonpath={..namespace}' 2>/dev/null || echo "default")
[ -z "$namespace" ] && namespace="default"

# ANSI colors
ESC=$(printf '\033')
CYAN="${ESC}[01;36m"
RESTORE="${ESC}[0m"

echo "K8s: ${CYAN}󱏚 ${RESTORE} $context ($namespace)"
